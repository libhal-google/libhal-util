#pragma once

#include <chrono>
#include <cmath>
#include <cstdint>
#include <ios>

#include <libhal/error.hpp>
#include <libhal/timeout.hpp>
#include <libhal/units.hpp>

#include "math.hpp"

namespace hal {
/**
 * @brief Calculate the number of cycles of this frequency within the time
 * duration. This function is meant for timers to determine how many count
 * cycles are needed to reach a particular time duration at this frequency.
 *
 * @param p_source - source frequency
 * @param p_duration - the amount of time to convert to cycles
 * @return std::int64_t - number of cycles
 */
[[nodiscard]] constexpr std::int64_t cycles_per(
  hertz p_source,
  hal::time_duration p_duration) noexcept
{
  // Full Equation:
  //                              / ratio_num \_
  //   frequency_hz * |period| * | ----------- |  = cycles
  //                              \ ratio_den /
  //
  // std::chrono::nanoseconds::period::num == 1
  // std::chrono::nanoseconds::period::den == 1,000,000

  const auto denominator = decltype(p_duration)::period::den;
  const auto float_count = static_cast<float>(p_duration.count());
  const auto cycle_count = (float_count * p_source) / denominator;

  return static_cast<std::int64_t>(cycle_count);
}

/**
 * @brief Calculates and returns the wavelength in seconds.
 *
 * @tparam Period - desired period (defaults to std::femto for femtoseconds).
 * @param p_source - source frequency to convert to wavelength
 * @return std::chrono::duration<int64_t, Period> - time based wavelength of
 * the frequency.
 */
template<typename Period>
constexpr std::chrono::duration<int64_t, Period> wavelength(hertz p_source)
{
  using duration = std::chrono::duration<int64_t, Period>;

  static_assert(Period::num == 1, "The period ratio numerator must be 1");
  static_assert(Period::den >= 1,
                "The period ratio denominator must be 1 or greater than 1.");

  constexpr auto denominator = static_cast<decltype(p_source)>(Period::den);
  auto period = (1.0f / p_source) * denominator;

  if (std::isinf(period)) {
    return duration(std::numeric_limits<int64_t>::max());
  }

  return duration(static_cast<int64_t>(period));
}

/**
 * @brief Calculates and returns the wavelength in seconds as a float.
 *
 * @tparam float_t - float type
 * @tparam Period - desired period
 * @param p_source - source frequency to convert to wavelength
 * @return constexpr float - float representation of the time based wavelength
 * of the frequency.
 */
constexpr float wavelength(hertz p_source)
{
  if (equals(p_source, 0.0f)) {
    return 0.0f;
  }
  auto duration = (1.0f / p_source);
  return float(duration);
}

/**
 * @brief Calculate the amount of time it takes a frequency to oscillate a
 * number of cycles.
 *
 * @param p_source - the frequency to compute the cycles from
 * @param p_cycles - number of cycles within the time duration
 * @return std::chrono::nanoseconds - time duration based on this frequency
 * and the number of cycles
 */
[[nodiscard]] inline result<std::chrono::nanoseconds> duration_from_cycles(
  hertz p_source,
  uint32_t p_cycles) noexcept
{
  // Full Equation (based on the equation in cycles_per()):
  //
  //
  //                /    cycles * ratio_den    \_
  //   |period| =  | ---------------------------|
  //                \ frequency_hz * ratio_num /
  //
  constexpr auto ratio_den = std::chrono::nanoseconds::period::den;
  constexpr auto ratio_num = std::chrono::nanoseconds::period::num;
  constexpr auto int_min = std::numeric_limits<std::int64_t>::min();
  constexpr auto int_max = std::numeric_limits<std::int64_t>::max();
  constexpr auto float_int_min = static_cast<float>(int_min);
  constexpr auto float_int_max = static_cast<float>(int_max);

  const auto source = std::abs(p_source);
  const auto float_cycles = static_cast<float>(p_cycles);
  const auto nanoseconds = (float_cycles * ratio_den) / (source * ratio_num);

  if (float_int_min <= nanoseconds && nanoseconds <= float_int_max) {
    return std::chrono::nanoseconds(static_cast<std::int64_t>(nanoseconds));
  }
  return new_error(std::errc::result_out_of_range);
}

/**
 * @brief print byte type using ostreams
 *
 * Meant for unit testing, testing and simulation purposes
 * C++ streams, in general, should not be used for any embedded project that
 * will ever have to be used on an MCU due to its memory cost.
 *
 * @tparam CharT - character type
 * @tparam Traits - ostream traits type
 * @param p_ostream - the ostream
 * @param p_byte - object to convert to a string
 * @return std::basic_ostream<CharT, Traits>& - reference to the ostream
 */
template<class CharT, class Traits>
inline std::basic_ostream<CharT, Traits>& operator<<(
  std::basic_ostream<CharT, Traits>& p_ostream,
  const hal::byte& p_byte)
{
  return p_ostream << std::hex << "0x" << unsigned(p_byte);
}

/**
 * @brief concept for enumeration types
 *
 * @tparam T - enum type
 */
template<typename T>
concept enumeration = std::is_enum_v<T>;

[[nodiscard]] constexpr auto value(enumeration auto p_enum_value) noexcept
{
  return static_cast<std::underlying_type_t<decltype(p_enum_value)>>(
    p_enum_value);
}
}  // namespace hal
