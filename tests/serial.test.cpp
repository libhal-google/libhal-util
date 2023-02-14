#include <libhal-util/serial.hpp>

#include <algorithm>
#include <array>
#include <span>

#include <libhal-util/comparison.hpp>

#include <boost/ut.hpp>

namespace hal {
void serial_util_test()
{
  using namespace boost::ut;

  static constexpr hal::byte write_failure_byte{ 'C' };
  static constexpr hal::byte filler_byte{ 'A' };

  class fake_serial : public hal::serial
  {
  public:
    status driver_configure(const settings&) override
    {
      return {};
    }

    result<write_t> driver_write(std::span<const hal::byte> p_data) override
    {
      write_call_count++;
      if (p_data[0] == write_failure_byte) {
        return hal::new_error();
      }
      m_out = p_data;

      if (single_byte_out) {
        return write_t{ p_data.subspan(0, 1) };
      }
      return write_t{ p_data };
    }

    result<read_t> driver_read(std::span<hal::byte> p_data) override
    {
      if (p_data.size() == 0) {
        return read_t{
          .data = p_data,
          .available = 1,
          .capacity = 1,
        };
      }

      read_was_called = true;

      if (read_fails) {
        return hal::new_error();
      }

      // only fill 1 byte at a time
      p_data[0] = filler_byte;

      return read_t{
        .data = p_data.subspan(0, 1),
        .available = 1,
        .capacity = 1,
      };
    }

    result<flush_t> driver_flush() override
    {
      flush_called = true;
      return flush_t{};
    }

    virtual ~fake_serial()
    {
    }

    std::span<const hal::byte> m_out{};
    int write_call_count = 0;
    bool read_was_called = false;
    bool flush_called = false;
    bool read_fails = false;
    bool single_byte_out = false;
  };

  "operator==(serial::settings)"_test = []() {
    serial::settings a{};
    serial::settings b{};

    expect(a == b);
  };

  "operator!=(serial::settings)"_test = []() {
    serial::settings a{ .baud_rate = 9600 };
    serial::settings b{ .baud_rate = 1200 };

    expect(a != b);
  };

  "serial/util"_test = []() {
    "[success] write_partial full"_test = []() {
      // Setup
      fake_serial serial;
      const std::array<hal::byte, 4> expected_payload{};

      // Exercise
      auto result = write_partial(serial, expected_payload);

      // Verify
      expect(bool{ result });
      expect(result.value().data.size() == expected_payload.size());
      expect(!serial.flush_called);
      expect(that % expected_payload.data() == serial.m_out.data());
      expect(that % expected_payload.size() == serial.m_out.size());
      expect(that % !serial.read_was_called);
    };

    "[success] write_partial single byte at a time"_test = []() {
      // Setup
      fake_serial serial;
      const std::array<hal::byte, 4> expected_payload{};
      serial.single_byte_out = true;

      // Exercise
      auto result = write_partial(serial, expected_payload);

      // Verify
      expect(bool{ result });
      expect(1 == result.value().data.size());
      expect(!serial.flush_called);
      expect(that % &expected_payload[0] == serial.m_out.data());
      expect(that % 4 == serial.m_out.size());
      expect(that % !serial.read_was_called);
    };

    "[failure] write_partial"_test = []() {
      // Setup
      fake_serial serial;
      const std::array<hal::byte, 4> expected_payload{ write_failure_byte };

      // Exercise
      auto result = write_partial(serial, expected_payload);

      // Verify
      expect(!bool{ result });
      expect(!serial.flush_called);
      expect(that % nullptr == serial.m_out.data());
      expect(that % 0 == serial.m_out.size());
      expect(that % !serial.read_was_called);
    };

    "[success] write"_test = []() {
      // Setup
      fake_serial serial;
      const std::array<hal::byte, 4> expected_payload{};
      serial.single_byte_out = true;

      // Exercise
      auto result = write(serial, expected_payload);

      // Verify
      expect(bool{ result });
      expect(!serial.flush_called);
      expect(that % 1 == serial.m_out.size());
      expect(that % expected_payload.size() == serial.write_call_count);
      expect(that % !serial.read_was_called);
    };

    "[success] write(std::string_view)"_test = []() {
      // Setup
      fake_serial serial;
      std::string_view expected_payload = "abcd";
      serial.single_byte_out = true;

      // Exercise
      auto result = write(serial, expected_payload);

      // Verify
      expect(bool{ result });
      expect(!serial.flush_called);
      expect(that % expected_payload.end()[-1] == serial.m_out[0]);
      expect(that % 1 == serial.m_out.size());
      expect(that % expected_payload.size() == serial.write_call_count);
      expect(that % !serial.read_was_called);
    };

    "[success] read"_test = []() {
      // Setup
      fake_serial serial;
      std::array<hal::byte, 4> expected_buffer;
      expected_buffer.fill(filler_byte);
      std::array<hal::byte, 4> actual_buffer;

      // Exercise
      auto result = read(serial, actual_buffer, never_timeout());

      // Verify
      expect(result.has_value());
      expect(!serial.flush_called);
      expect(that % nullptr == serial.m_out.data());
      expect(that % 0 == serial.m_out.size());
      // bool test = expected_buffer == result.value();
      // expect(that % test) << expected_buffer << " != " << result.value();
    };

    "[failure read] read"_test = []() {
      // Setup
      fake_serial serial;
      std::array<hal::byte, 4> expected_buffer;
      serial.read_fails = true;

      // Exercise
      auto result = read(serial, expected_buffer, never_timeout());

      // Verify
      expect(!result.has_value());
      expect(!serial.flush_called);
      expect(that % serial.read_was_called);
      expect(that % nullptr == serial.m_out.data());
      expect(that % 0 == serial.m_out.size());
    };

    "[success] read<Length>"_test = []() {
      // Setup
      fake_serial serial;
      std::array<hal::byte, 5> expected_buffer;
      expected_buffer.fill(filler_byte);

      // Exercise
      auto result = read<expected_buffer.size()>(serial, never_timeout());

      // Verify
      expect(result.has_value());
      expect(!serial.flush_called);
      expect(that % expected_buffer == result.value());
      expect(that % serial.read_was_called);
      expect(that % nullptr == serial.m_out.data());
      expect(that % 0 == serial.m_out.size());
    };

    "[failure read] read<Length>"_test = []() {
      // Setup
      fake_serial serial;
      serial.read_fails = true;

      // Exercise
      auto result = read<5>(serial, never_timeout());

      // Verify
      expect(!result.has_value());
      expect(!serial.flush_called);
      expect(that % serial.read_was_called);
      expect(that % nullptr == serial.m_out.data());
      expect(that % 0 == serial.m_out.size());
    };

    "[success] write_then_read"_test = []() {
      // Setup
      fake_serial serial;
      const std::array<hal::byte, 4> expected_payload{};
      std::array<hal::byte, 4> expected_buffer;
      expected_buffer.fill(filler_byte);
      std::array<hal::byte, 4> actual_buffer;

      // Exercise
      auto result = write_then_read(
        serial, expected_payload, actual_buffer, never_timeout());

      // Verify
      expect(result.has_value());
      expect(!serial.flush_called);
      expect(that % expected_payload.data() == serial.m_out.data());
      expect(that % expected_payload.size() == serial.m_out.size());
      expect(that % expected_buffer == actual_buffer);
    };

    "[failure read] write_then_read"_test = []() {
      // Setup
      fake_serial serial;
      const std::array<hal::byte, 4> expected_payload{};
      std::array<hal::byte, 4> expected_buffer;
      expected_buffer.fill(filler_byte);
      std::array<hal::byte, 4> actual_buffer;
      serial.read_fails = true;

      // Exercise
      auto result = write_then_read(
        serial, expected_payload, actual_buffer, never_timeout());

      // Verify
      expect(!result.has_value());
      expect(!serial.flush_called);
      expect(that % serial.read_was_called);
      expect(that % expected_payload.data() == serial.m_out.data());
      expect(that % expected_payload.size() == serial.m_out.size());
      expect(that % expected_buffer != actual_buffer);
    };

    "[failure on write] write_then_read"_test = []() {
      // Setup
      fake_serial serial;
      const std::array<hal::byte, 4> expected_payload{ write_failure_byte };
      std::array<hal::byte, 4> expected_buffer;

      // Exercise
      auto result = write_then_read(
        serial, expected_payload, expected_buffer, never_timeout());

      // Verify
      expect(!result.has_value());
      expect(!serial.flush_called);
      expect(that % !serial.read_was_called);
      expect(that % nullptr == serial.m_out.data());
      expect(that % 0 == serial.m_out.size());
    };

    "[success] write_then_read<Length>"_test = []() {
      // Setup
      fake_serial serial;
      const std::array<hal::byte, 4> expected_payload{};
      std::array<hal::byte, 5> expected_buffer;
      expected_buffer.fill(filler_byte);

      // Exercise
      auto result =
        write_then_read<5>(serial, expected_payload, never_timeout());

      auto actual_array = result.value();

      // Verify
      expect(result.has_value());
      expect(!serial.flush_called);
      expect(that % expected_payload.data() == serial.m_out.data());
      expect(that % expected_payload.size() == serial.m_out.size());
      expect(serial.read_was_called);
      expect(that % expected_buffer == actual_array);
    };

    "[failure on write] write_then_read<Length>"_test = []() {
      // Setup
      fake_serial serial;
      const std::array<hal::byte, 4> expected_payload{ write_failure_byte };

      // Exercise
      auto result =
        write_then_read<5>(serial, expected_payload, never_timeout());

      // Verify
      expect(!result.has_value());
      expect(!serial.flush_called);
      expect(that % !serial.read_was_called);
      expect(that % nullptr == serial.m_out.data());
      expect(that % 0 == serial.m_out.size());
    };
  };

  struct save_serial_write : public hal::serial
  {
    status driver_configure(const settings&) override
    {
      return {};
    }

    result<write_t> driver_write(std::span<const hal::byte> p_data) override
    {
      m_out.assign(p_data.begin(), p_data.end());
      return write_t{ p_data };
    }

    result<read_t> driver_read(std::span<hal::byte>) override
    {
      return hal::new_error();
    }

    result<flush_t> driver_flush() override
    {
      return flush_t{};
    }

    virtual ~save_serial_write()
    {
    }

    std::vector<hal::byte> m_out{};
  };

  "print()"_test = []() {
    "print()"_test = []() {
      // Setup
      save_serial_write serial;
      const std::string_view expected_payload = "hello, world!";

      // Exercise
      print(serial, expected_payload);

      // Verify
      expect(
        that % expected_payload ==
        std::string_view(reinterpret_cast<const char*>(serial.m_out.data()),
                         serial.m_out.size()));
    };

    "[printf style 1] print()"_test = []() {
      // Setup
      save_serial_write serial;
      const std::string_view expected_payload = "hello 5";

      // Exercise
      print<128>(serial, "hello %d", 5);

      // Verify
      expect(
        that % expected_payload ==
        std::string_view(reinterpret_cast<const char*>(serial.m_out.data()),
                         serial.m_out.size()));
    };

    "[printf style 2] print()"_test = []() {
      // Setup
      save_serial_write serial;
      const std::string_view expected_payload = "hello 5 0xABCDEF";

      // Exercise
      print<128>(serial, "hello %d 0x%06X", 5, 0xABCDEF);

      // Verify
      expect(
        that % expected_payload ==
        std::string_view(reinterpret_cast<const char*>(serial.m_out.data()),
                         serial.m_out.size()));
    };

    "[printf style 3 trimmed] print()"_test = []() {
      // Setup
      save_serial_write serial;
      const std::string_view expected_payload = "hello 5\0";

      // Exercise
      print<8>(serial, "hello %d 0x%06X", 5, 0xABCDEF);

      // Verify
      expect(
        that % expected_payload ==
        std::string_view(reinterpret_cast<const char*>(serial.m_out.data()),
                         serial.m_out.size()));
    };
  };
};
}  // namespace hal
