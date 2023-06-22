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

  "hal::work_state helper functions"_test = []() {
    expect(that % false == hal::terminated(work_state::in_progress));
    expect(that % true == hal::terminated(work_state::failed));
    expect(that % true == hal::terminated(work_state::finished));

    expect(that % true == hal::in_progress(work_state::in_progress));
    expect(that % false == hal::in_progress(work_state::failed));
    expect(that % false == hal::in_progress(work_state::finished));

    expect(that % false == hal::finished(work_state::in_progress));
    expect(that % false == hal::finished(work_state::failed));
    expect(that % true == hal::finished(work_state::finished));

    expect(that % false == hal::failed(work_state::in_progress));
    expect(that % true == hal::failed(work_state::failed));
    expect(that % false == hal::failed(work_state::finished));
  };

  "hal::work_state helper functions"_test = []() {
    // Setup
    struct always_in_progress
    {
      work_state state()
      {
        return work_state::in_progress;
      }
    };
    struct always_failed
    {
      work_state state()
      {
        return work_state::failed;
      }
    };
    struct always_finished
    {
      work_state state()
      {
        return work_state::finished;
      }
    };

    always_in_progress always_in_progress_obj;
    always_failed always_failed_obj;
    always_finished always_finished_obj;

    // Exercise + Verify
    expect(that % false == hal::terminated(always_in_progress_obj));
    expect(that % true == hal::terminated(always_failed_obj));
    expect(that % true == hal::terminated(always_finished_obj));

    expect(that % true == hal::in_progress(always_in_progress_obj));
    expect(that % false == hal::in_progress(always_failed_obj));
    expect(that % false == hal::in_progress(always_finished_obj));

    expect(that % false == hal::finished(always_in_progress_obj));
    expect(that % false == hal::finished(always_failed_obj));
    expect(that % true == hal::finished(always_finished_obj));

    expect(that % false == hal::failed(always_in_progress_obj));
    expect(that % true == hal::failed(always_failed_obj));
    expect(that % false == hal::failed(always_finished_obj));
  };
};
}  // namespace hal
