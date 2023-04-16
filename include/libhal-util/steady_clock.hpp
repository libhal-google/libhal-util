// Copyright 2023 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <chrono>
#include <system_error>

#include <libhal/error.hpp>
#include <libhal/steady_clock.hpp>
#include <libhal/timeout.hpp>

#include "units.hpp"

namespace hal {
/**
 * @brief Timeout object based on hal::steady_clock
 *
 * Do not use this class directly. Use `hal::create_timeout(hal::steady_clock&)`
 * instead of instantiating this class directly.
 *
 */
class steady_clock_timeout
{
public:
  /**
   * @brief Create a steady_clock_timeout
   *
   * @return result<steady_clock_timeout> - steady_clock_timeout object
   * @throws std::errc::result_out_of_range if time duration is zero or negative
   */
  static result<steady_clock_timeout> create(hal::steady_clock& p_steady_clock,
                                             hal::time_duration p_duration)
  {
    using period = decltype(p_duration)::period;
    const auto frequency =
      HAL_CHECK(p_steady_clock.frequency()).operating_frequency;
    const auto tick_period = wavelength<period>(frequency);
    auto ticks_required = p_duration / tick_period;
    using unsigned_ticks = std::make_unsigned_t<decltype(ticks_required)>;

    if (ticks_required <= 1) {
      ticks_required = 1;
    }

    const auto ticks = static_cast<unsigned_ticks>(ticks_required);
    const auto future_timestamp =
      ticks + HAL_CHECK(p_steady_clock.uptime()).ticks;

    return steady_clock_timeout(p_steady_clock, future_timestamp);
  }

  /**
   * @brief Construct a new counter timeout object
   *
   * @param p_timeout - other steady_clock_timeout
   */
  steady_clock_timeout(const steady_clock_timeout& p_timeout) = default;
  /**
   * @brief Assign construct a new counter timeout object
   *
   * @param p_timeout - other steady_clock_timeout
   * @return steady_clock_timeout&
   */
  steady_clock_timeout& operator=(const steady_clock_timeout& p_timeout) =
    default;
  /**
   * @brief Construct a new counter timeout object
   *
   * @param p_timeout - other steady_clock_timeout
   */
  steady_clock_timeout(steady_clock_timeout&& p_timeout) = default;
  /**
   * @brief Move assign construct a new counter timeout object
   *
   * @param p_timeout - other steady_clock_timeout
   * @return steady_clock_timeout&
   */
  steady_clock_timeout& operator=(steady_clock_timeout&& p_timeout) = default;

  /**
   * @brief Call this object to check if it has timed out.
   *
   * @return status - success or failure
   * @throws std::errc::timed_out - if the timeout time has been exceeded.
   */
  status operator()()
  {
    auto current_count = HAL_CHECK(m_counter->uptime()).ticks;

    if (current_count >= m_cycles_until_timeout) {
      return hal::new_error(std::errc::timed_out);
    }

    return success();
  }

private:
  /**
   * @brief Construct a new counter timeout object
   *
   * @param p_steady_clock - steady clock implementation
   * @param p_cycles_until_timeout - number of cycles until timeout
   */
  steady_clock_timeout(hal::steady_clock& p_steady_clock,
                       std::uint64_t p_cycles_until_timeout)
    : m_counter(&p_steady_clock)
    , m_cycles_until_timeout(p_cycles_until_timeout)
  {
  }

  hal::steady_clock* m_counter;
  std::uint64_t m_cycles_until_timeout = 0;
};

/**
 * @brief Create a timeout object based on hal::steady_clock.
 *
 * NOTE: Multiple timeout objects can be made from a single steady_clock
 * without influencing other timeout objects.
 *
 * @param p_steady_clock - hal::steady_clock implementation
 * @param p_duration - amount of time until timeout
 * @return result<hal::steady_clock_timeout> - timeout object
 */
inline result<steady_clock_timeout> create_timeout(
  hal::steady_clock& p_steady_clock,
  hal::time_duration p_duration)
{
  return steady_clock_timeout::create(p_steady_clock, p_duration);
}

/**
 * @brief Delay execution for a duration of time using a hardware steady_clock.
 *
 * @param p_steady_clock - steady_clock driver
 * @param p_duration - the amount of time to delay for. Zero or negative time
 * duration will delay for one tick of the p_steady_clock.
 * @return status - if successful, then execution of the program was halted for
 * the p_duration time frame, otherwise, a failure occurred either when creating
 * the timeout from the steady clock or from calling uptime() on the steady
 * clock. In the failure situation, it is unknown how long the program/thread
 * halted for.
 */
[[nodiscard]] inline status delay(hal::steady_clock& p_steady_clock,
                                  hal::time_duration p_duration)
{
  auto timeout_object = HAL_CHECK(create_timeout(p_steady_clock, p_duration));
  return hal::delay(timeout_object);
}

/**
 * @brief Generates a function that, when passed a duration, returns a timeout
 *
 * @param p_steady_clock - steady_clock driver that must out live the lifetime
 * of the returned lambda.
 * @return auto - a callable that returns a new timeout object each time a time
 * duration is passed to it.
 */
inline auto timeout_generator(hal::steady_clock& p_steady_clock)
{
  return [&p_steady_clock](hal::time_duration p_duration) -> auto
  {
    return create_timeout(p_steady_clock, p_duration);
  };
}
}  // namespace hal
