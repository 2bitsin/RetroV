#pragma once

#include <capstone/capstone.h>
#include <capstone/x86.h>

#include <functional>
#include <cstdint>
#include <cstddef>
#include <stdexcept>
#include <memory>
#include <tuple>
#include <span>
#include <cassert>
#include <variant>

namespace capstone
{
	struct error : std::exception
	{
		error (cs_err err, std::string_view str = ""): m_err { err }, m_str { str } {}
		error (csh handle, std::string_view str = ""): error (cs_errno (handle), str) {}
	 ~error () override = default;

		error (const error&) = default;
		error& operator = (const error&) = default;
		error (error&&) noexcept = default;
		error& operator = (error&&) noexcept = default;

		auto operator <=> (const error&) const noexcept = default;

		auto code () const noexcept { return m_err; }
		auto what () const noexcept -> const char* override { return m_str.c_str(); }

	private:
		cs_err m_err;
		std::string m_str;
	};	

	struct instance;

	struct x86_operand_view 
	: public ::cs_x86_op
	{
		using super = ::cs_x86_op;
		
	};

	struct x86_instruction_view
	: protected ::cs_insn
	{
		using super = ::cs_insn;
		using operand_type = std::variant<std::monostate>;

		auto operands_string () const noexcept 
			-> std::string_view
		{ return super::op_str; }

		auto mnemonic_string () const noexcept 
			-> std::string_view
		{ return super::mnemonic; }

		auto address () const noexcept 
			-> std::uint64_t 
		{ return super::address; }

		auto operands() const noexcept
			-> std::span<const x86_operand_view>
		{			
			return { (const x86_operand_view*)&_detail_x86().operands[0],
				_detail_x86().op_count };
		}

		auto operands (size_t index) const
			-> x86_operand_view const&
		{ 
			if (operands().size () <= index)
				throw std::out_of_range ("Operand index out of range!");
			return operands() [index]; 
		}

		auto size() const noexcept
		{
			return super::size;
		}

		auto bytes() const noexcept 
			-> std::span<std::uint8_t const>
		{
			return{ super::bytes, super::size };
		}

		auto groups() const noexcept 
			-> std::span<uint8_t const>
		{
			
			return { _detail().groups, 
				_detail().groups_count };
		}

	private:
		auto _detail() const -> const ::cs_detail&
		{
			assert(super::detail != nullptr);
			if (!super::detail)
				throw std::logic_error("Opcode details not present, please enable CS_OPT_DETAIL option.");
			return *super::detail;			
		}

		auto _detail_x86() const -> const ::cs_x86&
		{
			return _detail().x86;
		}

	};

	struct assembly
	:	public std::span<const x86_instruction_view>
	{
		using super = std::span<const x86_instruction_view>;
		friend struct instance;	

		assembly(const assembly&) = delete;
		assembly& operator = (const assembly&) = delete;

		assembly(assembly&& from) noexcept
		: super { std::exchange ((super&)from, super{}) }			
		{}

		assembly& operator = (assembly&& from) noexcept
		{
			auto tmp { std::move (from) };
			tmp.swap(*this);
			return *this;
		}

		void swap(assembly& with) noexcept
		{
			std::swap((super&)*this, (super&)with);
		}

		assembly(): super {} {}

		~assembly()
		{
			if (super::data() != nullptr)
			{
				::cs_free((::cs_insn*)super::data(), super::size());
				((super&)*this) = super{};
			}
		}

	private:
		assembly(const cs_insn* data, size_t size) noexcept
		:	super { (const x86_instruction_view*)data, size }
		{}
	};

	struct instance
	{			
		using sd_function = std::function<std::size_t (std::uint64_t, std::span<std::uint8_t const>)>;

		instance (::cs_arch arch, ::cs_mode mode, 
			std::initializer_list<std::tuple<::cs_opt_type, size_t>> opts)
		:	m_value { 0u }
		{
			auto _cs_error = ::cs_open(arch, mode, &m_value);
			assert (_cs_error == CS_ERR_OK);
			if (CS_ERR_OK != _cs_error) {
				throw capstone::error (_cs_error, 
					"Unable to open capstone handle.");
			}
			assert(m_value != 0u);
			for (auto[o_key, o_val] : opts)
			{
				_cs_error = ::cs_option(m_value, o_key, o_val);
				assert (_cs_error == CS_ERR_OK);
				if (_cs_error != CS_ERR_OK)
					throw capstone::error (_cs_error, 
						"Unable to open capstone handle.");
			}
		}

		instance (const instance&) = delete;
		instance& operator = (const instance&) = delete;

		instance(instance&& from) noexcept
		:	m_value { std::exchange (from.m_value, 0u) }
		{}

		instance& operator = (instance&& from) noexcept
		{
			instance tmp { std::move (from) };	
			std::swap(tmp, *this);
			return *this;
		}

		void swap(instance& with) noexcept
		{
			std::swap (m_value, with.m_value);
		}

	 ~instance () noexcept
		{
			if (m_value != 0u)
				cs_close (&m_value);
			m_value = 0u;
		}

	  auto value () const noexcept { return m_value ; }

		auto disasm(std::span<const std::byte> code, std::uint64_t base = 0u, std::size_t count = 0u)
			-> assembly
		{
			cs_insn* insn_ptr { nullptr };

			const auto length = cs_disasm (value (), (std::uint8_t const*)code.data (), code.size (), base, count, &insn_ptr);
			if (length < 1) 
			{
				throw capstone::error (value(), "Unable to disassemble code.");
			}
			assert (insn_ptr != nullptr);
			return assembly { insn_ptr, length };
		}

		auto register_name(x86_reg what) const
			-> std::string
		{
			auto result = ::cs_reg_name(value(), what);
			if (result != nullptr)
				return result;
			throw error(value(), "Unable to get register name.");
		}

		template <typename F>
		requires (std::is_invocable_r_v<std::size_t, F, std::uint64_t, std::span<std::uint8_t const>>)
		inline auto skipdata(std::string_view mnemonic_v, F&& callback_v) 
		{
			m_sdcallback = std::forward<F>(callback_v);
			cs_opt_skipdata sd_v { 
				.mnemonic = mnemonic_v.data(),
				.callback = [] (auto data_v, auto size_v, auto offset_v, auto user_p) 
				{
					return (*(instance*)user_p).m_sdcallback(offset_v, {data_v, size_v}); 
				},
				.user_data = this
			};
			
			::cs_option(value(), CS_OPT_SKIPDATA, CS_OPT_ON);
			::cs_option(value(), CS_OPT_SKIPDATA_SETUP, (std::size_t)&sd_v);
		}

	private:
		::csh m_value;	
		sd_function m_sdcallback;
		
	};
}
