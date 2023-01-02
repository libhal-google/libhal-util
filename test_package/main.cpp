#include <cstdio>
#include <numeric>

#include <libhal-util/i2c.hpp>

class test_i2c : public hal::i2c
{
private:
  hal::status driver_configure([[maybe_unused]] const settings& p_settings)
  {
    return hal::success();
  }
  hal::status driver_transaction(
    [[maybe_unused]] hal::byte p_address,
    [[maybe_unused]] std::span<const hal::byte> p_data_out,
    std::span<hal::byte> p_data_in,
    [[maybe_unused]] hal::function_ref<hal::timeout_function> p_timeout)
  {
    std::iota(p_data_in.begin(), p_data_in.end(), 5);
    return hal::success();
  }
};

int main()
{
  test_i2c i2c;

  auto array = hal::read<5>(i2c, 0x11, hal::never_timeout()).value();

  std::printf("read content = { ");
  for (const auto& byte : array) {
    std::printf("0x%02X ", byte);
  }
  std::printf("};\n");

  return 0;
}
