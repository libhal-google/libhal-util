#pragma once

#include <libhal/interrupt_pin.hpp>

namespace hal {
[[nodiscard]] constexpr auto operator==(
  const interrupt_pin::settings& p_lhs,
  const interrupt_pin::settings& p_rhs) noexcept
{
  return p_lhs.resistor == p_rhs.resistor && p_lhs.trigger == p_rhs.trigger;
}
}  // namespace hal
