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

#include <libhal-util/steady_clock.hpp>

#include <boost/ut.hpp>

// #include <libhal/testing.hpp>

namespace hal {
void steady_clock_utility_test()
{
  using namespace boost::ut;

  // Make the frequency equal to inverse of the time duration period, giving you
  // the frequency of each tick of time_duration.
  static constexpr auto expected_frequency =
    static_cast<hertz>(hal::time_duration::period::den);

  class dummy_steady_clock : public hal::steady_clock
  {
  public:
    std::uint64_t m_uptime = 0;

  private:
    frequency_t driver_frequency() override
    {
      return frequency_t{ .operating_frequency = expected_frequency };
    }

    uptime_t driver_uptime() override
    {
      return uptime_t{ .ticks = m_uptime++ };
    }
  };

  // =============== timeout ===============

  "hal::create_timeout(hal::steady_clock, 0ns)"_test = []() {
    // Setup
    static constexpr hal::time_duration expected(0);
    dummy_steady_clock test_steady_clock;
    bool success = false;

    // Exercise
    auto timeout_object = create_timeout(test_steady_clock, expected);

    hal::attempt_all(
      [&timeout_object]() -> status { return timeout_object(); },
      [&success](match<std::errc, std::errc::timed_out>) { success = true; },
      []() { expect(false) << "std::errc::timed_out was not thrown!"; });

    // Verify
    expect(that % success) << "std::errc::timed_out handler was not called!";
    // Verify: subtract 2 because 2 invocations are required in order to get
    //         the start uptime and another to check what the latest uptime is.
    expect(that % expected.count() == (test_steady_clock.m_uptime - 2));
    expect(that % expected_frequency ==
           test_steady_clock.frequency().operating_frequency);
  };

  "hal::create_timeout(hal::steady_clock, 50ns)"_test = []() {
    // Setup
    static constexpr hal::time_duration expected(50);
    dummy_steady_clock test_steady_clock;
    bool success = false;

    // Exercise
    auto timeout_object = create_timeout(test_steady_clock, expected);

    hal::attempt_all(
      [&timeout_object]() -> status {
        // Terminate the loop one iteration before the timeout would occur
        for (std::int64_t i = 0; i < expected.count() - 1; i++) {
          if (!timeout_object()) {
            return hal::new_error();
          }
        }
        return timeout_object();
      },
      [&success](match<std::errc, std::errc::timed_out>) { success = true; },
      []() { expect(false) << "std::errc::timed_out was not thrown!"; });

    // Verify
    expect(that % success) << "std::errc::timed_out handler was not called!";
    // After the last call to uptime() the uptime value is incremented by one
    expect(that % expected.count() == test_steady_clock.m_uptime - 1);
    expect(that % expected_frequency ==
           test_steady_clock.frequency().operating_frequency);
  };

  "hal::create_timeout(hal::steady_clock, 1337ns)"_test = []() {
    // Setup
    static constexpr hal::time_duration expected(10);
    dummy_steady_clock test_steady_clock;
    bool success = false;

    // Exercise
    auto timeout_object = create_timeout(test_steady_clock, expected);

    hal::attempt_all(
      [&timeout_object]() -> status {
        for (std::int64_t i = 0; i < expected.count() - 1; i++) {
          // error out if this times out early
          if (!timeout_object()) {
            return hal::new_error();
          }
        }
        return timeout_object();
      },
      [&success](match<std::errc, std::errc::timed_out>) { success = true; },
      []() { expect(false) << "std::errc::timed_out was not thrown!"; });

    // Verify
    expect(that % success) << "std::errc::timed_out handler was not called!";
    expect(that % expected.count() == test_steady_clock.m_uptime - 1);
    expect(that % expected_frequency ==
           test_steady_clock.frequency().operating_frequency);
  };

  "hal::create_timeout(hal::steady_clock, -5ns) returns error"_test = []() {
    // Setup
    static constexpr hal::time_duration expected(-5);
    dummy_steady_clock test_steady_clock;

    // Exercise
    [[maybe_unused]] auto result = create_timeout(test_steady_clock, expected);

    // Verify
    expect(that % 1 == test_steady_clock.m_uptime);
    expect(that % expected_frequency ==
           test_steady_clock.frequency().operating_frequency);
  };

  // =============== delay ===============

  "hal::delay(hal::steady_clock, 0ns)"_test = []() {
    // Setup
    static constexpr hal::time_duration expected(0);
    dummy_steady_clock test_steady_clock;

    // Exercise
    delay(test_steady_clock, expected);

    // Verify
    // Verify: subtract 2 because 2 invocations are required in order to get
    //         the start uptime and another to check what the latest uptime is.
    expect(that % expected.count() == (test_steady_clock.m_uptime - 2));
    expect(that % expected_frequency ==
           test_steady_clock.frequency().operating_frequency);
  };

  "hal::delay(hal::steady_clock, 50ns)"_test = []() {
    // Setup
    static constexpr hal::time_duration expected(50);
    dummy_steady_clock test_steady_clock;

    // Exercise
    delay(test_steady_clock, expected);

    // Verify
    expect(that % expected.count() == test_steady_clock.m_uptime - 1);
    expect(that % expected_frequency ==
           test_steady_clock.frequency().operating_frequency);
  };

  "hal::delay(hal::steady_clock, 1337ns)"_test = []() {
    // Setup
    static constexpr hal::time_duration expected(1'337);
    dummy_steady_clock test_steady_clock;

    // Exercise
    delay(test_steady_clock, expected);

    // Verify
    expect(that % expected.count() == test_steady_clock.m_uptime - 1);
    expect(that % expected_frequency ==
           test_steady_clock.frequency().operating_frequency);
  };

  "hal::delay(hal::steady_clock, -5ns) returns error"_test = []() {
    // Setup
    static constexpr hal::time_duration expected(-5);
    dummy_steady_clock test_steady_clock;

    // Exercise
    delay(test_steady_clock, expected);

    // Verify
    // Verify: Adjust uptime by 2 because at least 2 calls to uptime()
    expect(that % 0 == test_steady_clock.m_uptime - 2);
    expect(that % expected_frequency ==
           test_steady_clock.frequency().operating_frequency);
  };

  "hal::timeout_generator(hal::steady_clock)"_test = []() {
    // Setup
    static constexpr hal::time_duration expected(50);
    dummy_steady_clock test_steady_clock;
    bool success = false;

    // Exercise
    auto generator = timeout_generator(test_steady_clock);
    auto timeout_object = generator(expected);

    hal::attempt_all(
      [&timeout_object]() -> status {
        // Terminate the loop one iteration before the timeout would occur
        for (std::int64_t i = 0; i < expected.count() - 1; i++) {
          if (!timeout_object()) {
            return hal::new_error();
          }
        }
        return timeout_object();
      },
      [&success](match<std::errc, std::errc::timed_out>) { success = true; },
      []() { expect(false) << "std::errc::timed_out was not thrown!"; });

    // Verify
    expect(that % success) << "std::errc::timed_out handler was not called!";
    // After the last call to uptime() the uptime value is incremented by one
    expect(that % expected.count() == test_steady_clock.m_uptime - 1);
    expect(that % expected_frequency ==
           test_steady_clock.frequency().operating_frequency);
  };
};
}  // namespace hal
