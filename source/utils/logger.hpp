#pragma once

#include <source_location>
#include <unordered_map>
#include <string_view>
#include <iostream>
#include <fstream>
#include <string>
#include <format>
#include <chrono>
#include <mutex>

namespace utils
{
	namespace detail
	{
		template<typename Ctype, size_t Size>
		struct cxstr {

			consteval cxstr(Ctype const (&value_v)[Size]) noexcept {
				for(auto i=0u;i<Size;++i)
					m_value[i]=value_v[i];
			}

			inline constexpr operator std::basic_string_view<Ctype> () const		 
			{
				return { m_value };
			}
						Ctype m_value [Size];
		};

		template <int Index, cxstr Name>
		struct logger_level {
			static inline constexpr const auto index = int(Index);
			static inline constexpr const auto value = std::string_view(Name);
		};

		using level_trace   = logger_level<-2, "TRACE">;
		using level_debug   = logger_level<-1, "DEBUG">;
		using level_info    = logger_level<+0, "INFO">;
		using level_warning = logger_level<+1, "WARNING">;
		using level_error   = logger_level<+2, "ERROR">;
		using level_fatal   = logger_level<+3, "FATAL">;

		static constexpr const auto cout_sink = [](auto&& level_v, auto&& what_v) { std::cout << what_v << std::endl; };
		static constexpr const auto cerr_sink = [](auto&& level_v, auto&& what_v) { std::cerr << what_v << std::endl; };

		extern "C" void __stdcall OutputDebugStringA(char const* string_v);
		static constexpr const auto wdbg_sink = [](auto&& level_v, auto&& what_v) { 						
			std::string temp_v = what_v;
			temp_v.append("\r\n");
			OutputDebugStringA(temp_v.c_str());
		};

		static inline constexpr const auto file_sink = [](auto&& level_v, auto&& what_v) {
			static std::unordered_map<std::string_view, std::ofstream> m_logs;
			static std::mutex m_mutex;
			std::lock_guard _{ m_mutex }; 
			if (!m_logs.contains(level_v.value)) {
				m_logs.emplace(level_v.value, std::ofstream(
					std::format("{}.log", level_v.value), std::ios::app));
			}
			auto& file_v = m_logs.at(level_v.value);
			file_v << what_v << std::endl;
		};

			 
		template<typename Level>
		struct logger_statement 
		{
			template<typename Sink, typename...Args>
			struct type {
				static inline constexpr const auto level = Level{};

				type(Sink&& sink_v, std::format_string<Args...> fmt_s, Args&&...args_v,
					std::source_location srcloc_v = std::source_location::current())
				{
					sink_v(level, std::format("[{:%d-%m-%Y %H:%M:%OS} {}] {}"
					#if 0
						" ({}:{}:{})"
					#endif
						,
						std::chrono::system_clock::now(), 
						level.value,
						std::format(fmt_s, std::forward<Args>(args_v)...)
					#if 0
						,
						srcloc_v.file_name(),
						srcloc_v.line(),
						srcloc_v.column()
					#endif
						));
				}
			};

			template <typename Sink, typename...Args>
			type(Sink&& sink_v, std::format_string<Args...> fmt_s, Args&&...args_v)
				-> type<Sink, Args...>;
		};
	}

	struct logger {
		using trace = typename detail::logger_statement<detail::level_trace  >::type;
		using debug = typename detail::logger_statement<detail::level_debug  >::type;
		using info  = typename detail::logger_statement<detail::level_info   >::type;
		using warn  = typename detail::logger_statement<detail::level_warning>::type;
		using error = typename detail::logger_statement<detail::level_error  >::type;
		using fatal = typename detail::logger_statement<detail::level_fatal  >::type;

		static inline auto const cat = []<typename...Sink>(Sink&&...sink_v) {
			return [&](auto&& level_v, auto&& what_v) {
				((sink_v(level_v, what_v)), ... );
			};
		};		

		static inline auto const& cerr = detail::cerr_sink;
		static inline auto const& cout = detail::cout_sink;
		static inline auto const& file = detail::file_sink;
		static inline auto const& wdbg = detail::wdbg_sink;

		static inline auto const deflog = logger::cat(logger::file, logger::cerr, logger::wdbg);

	};


}
