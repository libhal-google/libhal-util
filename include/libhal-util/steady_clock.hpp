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
    const auto frequency = p_steady_clock.frequency();
    const auto tick_period = wavelength<period>(frequency);
    auto ticks_required = p_duration / tick_period;
    using unsigned_ticks = std::make_unsigned_t<decltype(ticks_required)>;

    if (ticks_required <= 1) {
      ticks_required = 1;
    }

    const auto ticks = static_cast<unsigned_ticks>(ticks_required);
    const auto future_timestamp = ticks + HAL_CHECK(p_steady_clock.uptime());

    return steady_clock_timeout(p_steady_clock, future_timestamp);
  }

  /**
   * @brief Construct a new counter timeout object
   *
   * @param p_timeout - other steady_clock_timeout
   */
  steady_clock_timeout(const steady_clock_timeout& p_timeout) noexcept =
    default;
  /**
   * @brief Assign construct a new counter timeout object
   *
   * @param p_timeout - other steady_clock_timeout
   * @return steady_clock_timeout&
   */
  steady_clock_timeout& operator=(
    const steady_clock_timeout& p_timeout) noexcept = default;
  /**
   * @brief Construct a new counter timeout object
   *
   * @param p_timeout - other steady_clock_timeout
   */
  steady_clock_timeout(steady_clock_timeout&& p_timeout) noexcept = default;
  /**
   * @brief Move assign construct a new counter timeout object
   *
   * @param p_timeout - other steady_clock_timeout
   * @return steady_clock_timeout&
   */
  steady_clock_timeout& operator=(steady_clock_timeout&& p_timeout) noexcept =
    default;

  /**
   * @brief Call this object to check if it has timed out.
   *
   * @return status - success or failure
   * @throws std::errc::timed_out - if the timeout time has been exceeded.
   */
  status operator()() noexcept
  {
    auto current_count = HAL_CHECK(m_counter->uptime());

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
                       std::uint64_t p_cycles_until_timeout) noexcept
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
inline result<hal::steady_clock_timeout> create_timeout(
  hal::steady_clock& p_steady_clock,
  hal::time_duration p_duration)
{
  return hal::steady_clock_timeout::create(p_steady_clock, p_duration);
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
                                  hal::time_duration p_duration) noexcept
{
  auto timeout_object = HAL_CHECK(create_timeout(p_steady_clock, p_duration));
  return delay(timeout_object);
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
