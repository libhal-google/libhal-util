#pragma once

#include <ios>
#include <string_view>
#include <type_traits>

#include <libhal/units.hpp>

namespace hal {

/**
 * @brief Get string representation of the work state.
 *
 * @param p_state - work state
 * @return std::string_view - string representation
 */
constexpr std::string_view to_string(work_state p_state)
{
  switch (p_state) {
    case work_state::in_progress:
      return "in progress";
    case work_state::failed:
      return "failed";
    case work_state::finished:
      return "finished";
    default:
      return "unknown work state";
  }
}

/**
 * @brief Indicate if a work_state is terminal, meaning the worker has finished
 * its job.
 *
 * @param p_state - the work state to check
 * @return true - work state is either finished or failed
 * @return false - work state is still in progress
 */
constexpr bool terminated(work_state p_state)
{
  return p_state == work_state::finished || p_state == work_state::failed;
}

/**
 * @brief print work_state type using ostreams
 *
 * Meant for unit testing, testing and simulation purposes
 * C++ streams, in general, should not be used for any embedded project that
 * will ever have to be used on an MCU due to its memory cost.
 *
 * @tparam CharT - character type
 * @tparam Traits - ostream traits type
 * @param p_ostream - the ostream
 * @param p_state - object to convert to a string
 * @return std::basic_ostream<CharT, Traits>& - reference to the ostream
 */
template<class CharT, class Traits>
inline std::basic_ostream<CharT, Traits>& operator<<(
  std::basic_ostream<CharT, Traits>& p_ostream,
  const work_state& p_state)
{
  return p_ostream << to_string(p_state);
}

/**
 * @brief concept for enumeration types
 *
 * @tparam T - enum type
 */
template<typename T>
concept enumeration = std::is_enum_v<T>;

/**
 * @brief Helper function to convert an enum to its integral value
 *
 * @param p_enum_value - the enumeration you want to convert into an integral
 * value
 * @return constexpr auto - return the integral value of the enum with the same
 * type as the enumeration.
 */
[[nodiscard]] constexpr auto value(enumeration auto p_enum_value) noexcept
{
  return static_cast<std::underlying_type_t<decltype(p_enum_value)>>(
    p_enum_value);
}
}  // namespace hal
