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

#include <libhal-util/i2c.hpp>

#include <libhal/functional.hpp>

#include <boost/ut.hpp>

namespace hal {
void i2c_util_test()
{
  using namespace boost::ut;

  static constexpr hal::byte successful_address{ 0x15 };
  static constexpr hal::byte failure_address{ 0x33 };
  static constexpr hal::byte filler_byte{ 0xA5 };

  struct test_timeout_t
  {
    status operator()()
    {
      was_called = true;
      return {};
    }
    bool was_called = false;
  };

  class test_i2c : public hal::i2c
  {
  public:
    [[nodiscard]] status driver_configure(const settings&) override
    {
      return {};
    }
    [[nodiscard]] result<transaction_t> driver_transaction(
      hal::byte p_address,
      std::span<const hal::byte> p_out,
      std::span<hal::byte> p_in,
      hal::function_ref<hal::timeout_function> p_timeout) override
    {
      m_address = p_address;
      m_out = p_out;
      m_in = p_in;

      std::fill(m_in.begin(), m_in.end(), filler_byte);

      if (m_address == failure_address) {
        return hal::new_error();
      }

      (void)p_timeout();

      return transaction_t{};
    }

    virtual ~test_i2c()
    {
    }

    hal::byte m_address = hal::byte{ 0 };
    std::span<const hal::byte> m_out = std::span<const hal::byte>{};
    std::span<hal::byte> m_in = std::span<hal::byte>{};
  };

  "operator==(i2c::settings)"_test = []() {
    i2c::settings a{};
    i2c::settings b{};

    expect(a == b);
  };

  "operator!=(i2c::settings)"_test = []() {
    i2c::settings a{ .clock_rate = 100.0_kHz };
    i2c::settings b{ .clock_rate = 1200.0_kHz };

    expect(a != b);
  };

  "[success] write"_test = []() {
    // Setup
    test_i2c i2c;
    test_timeout_t test_timeout;
    const std::array<hal::byte, 4> expected_payload{};

    // Exercise
    auto result =
      write(i2c, successful_address, expected_payload, std::ref(test_timeout));
    bool successful = static_cast<bool>(result);

    // Verify
    expect(successful);
    expect(successful_address == i2c.m_address);
    expect(that % expected_payload.data() == i2c.m_out.data());
    expect(that % expected_payload.size() == i2c.m_out.size());
    expect(that % nullptr == i2c.m_in.data());
    expect(that % 0 == i2c.m_in.size());
    expect(that % true == test_timeout.was_called);
  };

  "[failure] write"_test = []() {
    // Setup
    test_i2c i2c;
    test_timeout_t test_timeout;
    const std::array<hal::byte, 4> expected_payload{};

    // Exercise
    auto result =
      write(i2c, failure_address, expected_payload, std::ref(test_timeout));
    bool successful = static_cast<bool>(result);

    // Verify
    expect(!successful);
    expect(failure_address == i2c.m_address);
    expect(that % expected_payload.data() == i2c.m_out.data());
    expect(that % expected_payload.size() == i2c.m_out.size());
    expect(that % nullptr == i2c.m_in.data());
    expect(that % 0 == i2c.m_in.size());
    expect(that % false == test_timeout.was_called);
  };

  "[success] read"_test = []() {
    // Setup
    test_i2c i2c;
    test_timeout_t test_timeout;
    std::array<hal::byte, 4> expected_buffer;

    // Exercise
    auto result =
      read(i2c, successful_address, expected_buffer, std::ref(test_timeout));
    bool successful = static_cast<bool>(result);

    // Verify
    expect(successful);
    expect(successful_address == i2c.m_address);
    expect(that % expected_buffer.data() == i2c.m_in.data());
    expect(that % expected_buffer.size() == i2c.m_in.size());
    expect(that % nullptr == i2c.m_out.data());
    expect(that % 0 == i2c.m_out.size());
    expect(that % true == test_timeout.was_called);
  };

  "[failure] read"_test = []() {
    // Setup
    test_i2c i2c;
    test_timeout_t test_timeout;
    std::array<hal::byte, 4> expected_buffer;

    // Exercise
    auto result =
      read(i2c, failure_address, expected_buffer, std::ref(test_timeout));
    bool successful = static_cast<bool>(result);

    // Verify
    expect(!successful);
    expect(failure_address == i2c.m_address);
    expect(that % expected_buffer.data() == i2c.m_in.data());
    expect(that % expected_buffer.size() == i2c.m_in.size());
    expect(that % nullptr == i2c.m_out.data());
    expect(that % 0 == i2c.m_out.size());
    expect(that % false == test_timeout.was_called);
  };

  "[success] read<Length>"_test = []() {
    // Setup
    test_i2c i2c;
    test_timeout_t test_timeout;
    std::array<hal::byte, 5> expected_buffer;
    expected_buffer.fill(filler_byte);

    // Exercise
    auto result = read<expected_buffer.size()>(
      i2c, successful_address, std::ref(test_timeout));
    bool successful = static_cast<bool>(result);

    // Verify
    expect(successful);
    expect(successful_address == i2c.m_address);
    expect(std::equal(
      expected_buffer.begin(), expected_buffer.end(), result.value().begin()));
    expect(that % nullptr == i2c.m_out.data());
    expect(that % 0 == i2c.m_out.size());
    expect(that % true == test_timeout.was_called);
  };

  "[failure] read<Length>"_test = []() {
    // Setup
    test_i2c i2c;
    test_timeout_t test_timeout;

    // Exercise
    auto result = read<5>(i2c, failure_address, never_timeout());
    bool successful = static_cast<bool>(result);

    // Verify
    expect(!successful);
    expect(failure_address == i2c.m_address);
    expect(that % nullptr == i2c.m_out.data());
    expect(that % 0 == i2c.m_out.size());
    expect(that % false == test_timeout.was_called);
  };

  "[success] write_then_read"_test = []() {
    // Setup
    test_i2c i2c;
    test_timeout_t test_timeout;
    const std::array<hal::byte, 4> expected_payload{};
    std::array<hal::byte, 4> expected_buffer;

    // Exercise
    auto result = write_then_read(i2c,
                                  successful_address,
                                  expected_payload,
                                  expected_buffer,
                                  std::ref(test_timeout));
    bool successful = static_cast<bool>(result);

    // Verify
    expect(successful);
    expect(successful_address == i2c.m_address);
    expect(that % expected_payload.data() == i2c.m_out.data());
    expect(that % expected_payload.size() == i2c.m_out.size());
    expect(that % expected_buffer.data() == i2c.m_in.data());
    expect(that % expected_buffer.size() == i2c.m_in.size());
    expect(that % true == test_timeout.was_called);
  };

  "[failure] write_then_read"_test = []() {
    // Setup
    test_i2c i2c;
    test_timeout_t test_timeout;
    const std::array<hal::byte, 4> expected_payload{};
    std::array<hal::byte, 4> expected_buffer;
    expected_buffer.fill(filler_byte);

    // Exercise
    auto result = write_then_read(i2c,
                                  failure_address,
                                  expected_payload,
                                  expected_buffer,
                                  std::ref(test_timeout));
    bool successful = static_cast<bool>(result);

    // Verify
    expect(!successful);
    expect(failure_address == i2c.m_address);
    expect(that % expected_payload.data() == i2c.m_out.data());
    expect(that % expected_payload.size() == i2c.m_out.size());
    expect(that % expected_buffer.data() == i2c.m_in.data());
    expect(that % expected_buffer.size() == i2c.m_in.size());
    expect(that % false == test_timeout.was_called);
  };

  "[success] write_then_read<Length>"_test = []() {
    // Setup
    test_i2c i2c;
    test_timeout_t test_timeout;
    const std::array<hal::byte, 4> expected_payload{};
    std::array<hal::byte, 4> expected_buffer{};
    expected_buffer.fill(filler_byte);

    // Exercise
    auto result = write_then_read<5>(
      i2c, successful_address, expected_payload, std::ref(test_timeout));
    bool successful = static_cast<bool>(result);
    auto actual_array = result.value();

    // Verify
    expect(successful);
    expect(successful_address == i2c.m_address);
    expect(that % expected_payload.data() == i2c.m_out.data());
    expect(that % expected_payload.size() == i2c.m_out.size());
    expect(std::equal(
      expected_buffer.begin(), expected_buffer.end(), actual_array.begin()));
    expect(that % true == test_timeout.was_called);
  };

  "[failure] write_then_read<Length>"_test = []() {
    // Setup
    test_i2c i2c;
    test_timeout_t test_timeout;
    const std::array<hal::byte, 4> expected_payload{};

    // Exercise
    auto result = write_then_read<5>(
      i2c, failure_address, expected_payload, std::ref(test_timeout));
    bool successful = static_cast<bool>(result);

    // Verify
    expect(!successful);
    expect(failure_address == i2c.m_address);
    expect(that % expected_payload.data() == i2c.m_out.data());
    expect(that % expected_payload.size() == i2c.m_out.size());
    expect(that % false == test_timeout.was_called);
  };

  "[success] probe(i2c&)"_test = []() {
    // Setup
    test_i2c i2c;

    // Exercise
    auto result = probe(i2c, successful_address);
    bool successful = static_cast<bool>(result);

    // Verify
    expect(successful);
    expect(successful_address == i2c.m_address);
    expect(that % 1 == i2c.m_in.size());
    expect(that % nullptr != i2c.m_in.data());
    expect(that % 0 == i2c.m_out.size());
    expect(that % nullptr == i2c.m_out.data());
  };

  "[failure] probe(i2c&)"_test = []() {
    // Setup
    test_i2c i2c;

    // Exercise
    auto result = probe(i2c, failure_address);
    bool successful = static_cast<bool>(result);

    // Verify
    expect(!successful);
    expect(failure_address == i2c.m_address);
    expect(that % 1 == i2c.m_in.size());
    expect(that % nullptr != i2c.m_in.data());
    expect(that % 0 == i2c.m_out.size());
    expect(that % nullptr == i2c.m_out.data());
  };

  "Use all APIs without timeout parameter"_test = []() {
    // Setup
    test_i2c i2c;

    const std::array<hal::byte, 4> write_data{};
    std::array<hal::byte, 4> read_data{};

    // Exercise
    auto result_write = write(i2c, successful_address, write_data);
    auto result_read = read(i2c, successful_address, read_data);
    auto result_wr =
      write_then_read(i2c, successful_address, write_data, read_data);
    auto result_read_buffer = read<2>(i2c, successful_address);
    auto result_wr_buffer =
      write_then_read<2>(i2c, successful_address, write_data);

    // Verify
    expect(bool{ result_read });
    expect(bool{ result_write });
    expect(bool{ result_wr });
    expect(bool{ result_read_buffer });
    expect(bool{ result_wr_buffer });
  };
};
}  // namespace hal
