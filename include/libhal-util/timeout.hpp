#pragma once

#include <functional>
#include <ios>
#include <system_error>

#include <libhal/error.hpp>
#include <libhal/timeout.hpp>

#include "units.hpp"

namespace hal {
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

constexpr bool terminated(work_state p_state)
{
  return p_state == work_state::finished || p_state == work_state::failed;
}

template<class CharT, class Traits>
inline std::basic_ostream<CharT, Traits>& operator<<(
  std::basic_ostream<CharT, Traits>& p_ostream,
  const work_state& p_state)
{
  return p_ostream << to_string(p_state);
}

/**
 * @brief Repeatedly call a worker function until it has reached a terminal
 * state or a timeout has been reached
 *
 * @param p_worker - worker function to repeatedly call
 * @param p_timeout - callable timeout object
 * @return result<work_state> - state of the worker function
 */
inline result<work_state> try_until(worker auto& p_worker,
                                    timeout auto p_timeout)
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
                                    timeout auto p_timeout)
{
  worker auto& worker = p_worker;
  return try_until(worker, p_timeout);
};
}  // namespace hal
