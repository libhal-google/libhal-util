#pragma once

#include <functional>

#include <libhal/error.hpp>
#include <libhal/i2c.hpp>
#include <libhal/units.hpp>

#include "math.hpp"

namespace hal {
[[nodiscard]] constexpr auto operator==(const i2c::settings& p_lhs,
                                        const i2c::settings& p_rhs)
{
  return equals(p_lhs.clock_rate, p_rhs.clock_rate);
}

/**
 * @brief write data to a target device on the i2c bus
 *
 * Shorthand for writing i2c.transfer(...) for write only operations
 *
 * @param p_i2c - i2c driver
 * @param p_address - target address
 * @param p_data_out - buffer of bytes to write to the target device
 * @param p_timeout - amount of time to execute the transaction
 * @return status - success or failure
 */
[[nodiscard]] inline status write(i2c& p_i2c,
                                  hal::byte p_address,
                                  std::span<const hal::byte> p_data_out,
                                  timeout auto p_timeout = hal::never_timeout())
{
  return p_i2c.transaction(
    p_address, p_data_out, std::span<hal::byte>{}, p_timeout);
}
/**
 * @brief read bytes from target device on i2c bus
 *
 * Shorthand for writing i2c.transfer(...) for read only operations
 *
 * @param p_i2c - i2c driver
 * @param p_address - target address
 * @param p_data_in - buffer to read bytes into from target device
 * @param p_timeout - amount of time to execute the transaction
 * @return status - success or failure
 */
[[nodiscard]] inline status read(i2c& p_i2c,
                                 hal::byte p_address,
                                 std::span<hal::byte> p_data_in,
                                 timeout auto p_timeout = hal::never_timeout())
{
  return p_i2c.transaction(
    p_address, std::span<hal::byte>{}, p_data_in, p_timeout);
}
/**
 * @brief return array of read bytes from target device on i2c bus
 *
 * Eliminates the need to create a buffer and pass it into the read function.
 *
 * @tparam BytesToRead - number of bytes to read
 * @param p_i2c - i2c driver
 * @param p_address - target address
 * @param p_timeout - amount of time to execute the transaction
 * @return result<std::array<hal::byte, BytesToRead>> - array of
 * bytes from target device or an error.
 */
template<size_t BytesToRead>
[[nodiscard]] result<std::array<hal::byte, BytesToRead>> read(
  i2c& p_i2c,
  hal::byte p_address,
  timeout auto p_timeout = hal::never_timeout())
{
  std::array<hal::byte, BytesToRead> buffer;
  HAL_CHECK(read(p_i2c, p_address, buffer, p_timeout));
  return buffer;
}
/**
 * @brief write and then read bytes from target device on i2c bus
 *
 * This API simply calls transaction. This API is here for consistency across
 * the other other communication protocols such as SPI and serial.
 *
 * @param p_i2c - i2c driver
 * @param p_address - target address
 * @param p_data_out - buffer of bytes to write to the target device
 * @param p_data_in - buffer to read bytes into from target device
 * @param p_timeout - amount of time to execute the transaction
 *
 * @return status - success or failure
 */
[[nodiscard]] inline status write_then_read(
  i2c& p_i2c,
  hal::byte p_address,
  std::span<const hal::byte> p_data_out,
  std::span<hal::byte> p_data_in,
  timeout auto p_timeout = hal::never_timeout())
{
  return p_i2c.transaction(p_address, p_data_out, p_data_in, p_timeout);
}
/**
 * @brief write and then return an array of read bytes from target device on i2c
 * bus
 *
 * Eliminates the need to create a buffer and pass it into the read function.
 *
 * @tparam BytesToRead - number of bytes to read after write
 * @param p_i2c - i2c driver
 * @param p_address - target address
 * @param p_data_out - buffer of bytes to write to the target device
 * @param p_timeout - amount of time to execute the transaction
 * @return result<std::array<hal::byte, BytesToRead>>
 */
template<size_t BytesToRead>
[[nodiscard]] result<std::array<hal::byte, BytesToRead>> write_then_read(
  i2c& p_i2c,
  hal::byte p_address,
  std::span<const hal::byte> p_data_out,
  timeout auto p_timeout = hal::never_timeout())
{
  std::array<hal::byte, BytesToRead> buffer;
  HAL_CHECK(write_then_read(p_i2c, p_address, p_data_out, buffer, p_timeout));
  return buffer;
}

/**
 * @brief probe the i2c bus to see if a device exists
 *
 * @param p_i2c - i2c driver
 * @param p_address - target address to probe for
 * @return status - success or failure
 */
[[nodiscard]] inline status probe(i2c& p_i2c, hal::byte p_address)
{
  // p_data_in: empty placeholder for transcation's data_in
  std::array<hal::byte, 1> data_in;

  // p_timeout: no timeout placeholder for transaction's p_timeout
  timeout auto timeout = hal::never_timeout();

  return p_i2c.transaction(p_address, std::span<hal::byte>{}, data_in, timeout);
}
}  // namespace hal
