#pragma once

#include <libhal/output_pin.hpp>

namespace hal {
[[nodiscard]] constexpr auto operator==(
  const output_pin::settings& p_lhs,
  const output_pin::settings& p_rhs) noexcept
{
  return p_lhs.resistor == p_rhs.resistor &&
         p_lhs.open_drain == p_rhs.open_drain;
}
}  // namespace hal
