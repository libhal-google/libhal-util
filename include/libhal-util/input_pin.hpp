#pragma once

#include <libhal/input_pin.hpp>

namespace hal {
[[nodiscard]] constexpr auto operator==(
  const input_pin::settings& p_lhs,
  const input_pin::settings& p_rhs) noexcept
{
  return p_lhs.resistor == p_rhs.resistor;
}
}  // namespace hal
