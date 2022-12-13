#include <libhal-util/timeout.hpp>

#include <boost/ut.hpp>

namespace hal {
void timeout_test()
{
  using namespace boost::ut;
  "hal::try_until(callback, timeout)"_test = []() {
    // Setup
    constexpr int timeout_call_limit = 10;
    int counts = 0;
    auto timeout_callback = [&counts]() mutable -> status {
      counts++;
      if (counts >= timeout_call_limit) {
        return hal::new_error(std::errc::timed_out);
      }
      return {};
    };
    std::function<result<work_state>()> callback = [&counts]() {
      if (counts >= 5) {
        return work_state::finished;
      }
      return work_state::in_progress;
    };

    // Exercise
    auto result = hal::try_until(callback, timeout_callback).value();

    // Verify
    expect(that % work_state::finished == result);
    expect(that % 5 == counts);
  };

  "hal::try_until(callback, timeout) timed out"_test = []() {
    // Setup
    constexpr int timeout_call_limit = 10;
    int counts = 0;
    auto timeout_callback = [&counts]() mutable -> status {
      counts++;
      if (counts >= timeout_call_limit) {
        return hal::new_error(std::errc::timed_out);
      }
      return {};
    };
    std::function<result<work_state>()> callback = [&counts]() {
      if (counts >= 11) {
        return work_state::finished;
      }
      return work_state::in_progress;
    };

    // Exercise
    auto result = hal::try_until(callback, timeout_callback);

    // Verify
    expect(!bool{ result });
    expect(that % 10 == counts);
  };

  "hal::try_until(callback, timeout) with never_timeout"_test = []() {
    // Setup
    int counts = 0;
    std::function<result<work_state>()> callback = [&counts]() {
      counts++;
      if (counts >= 5) {
        return work_state::finished;
      }
      return work_state::in_progress;
    };

    // Exercise
    auto result = hal::try_until(callback, never_timeout()).value();

    // Verify
    expect(that % work_state::finished == result);
    expect(that % 5 == counts);
  };

  "hal::try_until(callback, timeout) error in callback"_test = []() {
    // Setup
    constexpr int timeout_call_limit = 10;
    int counts = 0;
    auto timeout_callback = [&counts]() mutable -> status {
      counts++;
      if (counts >= timeout_call_limit) {
        return hal::new_error(std::errc::timed_out);
      }
      return {};
    };
    std::function<result<work_state>()> callback = []() {
      return hal::new_error(std::errc::resource_unavailable_try_again);
    };

    bool callback_error = false;
    auto error_catcher =
      [&callback_error](
        hal::match<std::errc, std::errc::resource_unavailable_try_again> p_errc)
      -> status {
      (void)p_errc;
      callback_error = true;
      return {};
    };

    // Exercise
    auto result = hal::attempt(
      [&callback, &timeout_callback]() -> status {
        while (true) {
          HAL_CHECK(hal::try_until(callback, timeout_callback));
        }
        // Unreachable!
        return {};
      },
      error_catcher);

    // Verify
    expect(that % true == callback_error);
  };
};
}  // namespace hal
