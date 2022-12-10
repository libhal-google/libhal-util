#pragma once

#include <functional>
#include <system_error>

#include <libhal/error.hpp>
#include <libhal/timeout.hpp>

#include "units.hpp"

namespace hal {
/**
 * @brief Repeatedly call a worker function until it has reached a terminal
 * state or a timeout has been reached
 *
 * @param p_worker - worker function to repeatedly call
 * @param p_timeout - callable timeout object
 * @return result<work_state> - state of the worker function
 */
inline result<work_state> try_until(worker auto& p_worker,
                                    timeout auto p_timeout) noexcept
{
  while (true) {
    auto state = HAL_CHECK(p_worker());
    if (hal::terminated(state)) {
      return state;
    }
    HAL_CHECK(p_timeout());
  }
  return new_error(std::errc::state_not_recoverable);
};

/**
 * @brief Repeatedly call a worker function until it has reached a terminal
 * state or a timeout has been reached
 *
 * @param p_worker - worker function to repeatedly call
 * @param p_timeout - callable timeout object
 * @return result<work_state> - state of the worker function
 */
inline result<work_state> try_until(worker auto&& p_worker,
                                    timeout auto p_timeout) noexcept
{
  worker auto& worker = p_worker;
  return try_until(worker, p_timeout);
};
}  // namespace hal
