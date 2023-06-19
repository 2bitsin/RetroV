#include <core/config.hpp>
#include <core/config_impl.hpp>

auto core::config::create() -> std::unique_ptr<config>
{
  return std::make_unique<config_impl>();
}
