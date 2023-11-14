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

#pragma once

#include <span>

#include <libhal/error.hpp>
#include <libhal/spi.hpp>
#include <libhal/units.hpp>

namespace hal {
[[nodiscard]] constexpr auto operator==(const spi::settings& p_lhs,
                                        const spi::settings& p_rhs)
{
  return p_lhs.clock_idles_high == p_rhs.clock_idles_high &&
         p_lhs.clock_rate == p_rhs.clock_rate &&
         p_lhs.data_valid_on_trailing_edge == p_rhs.data_valid_on_trailing_edge;
}

/**
 * @brief Write data to the spi bus, ignore data on the peripherals receive line
 *
 * This command is useful for spi operations where data, such as a command,
 * must be sent to a device and the device does not respond with anything or the
 * response is not necessary.
 *
 * @param p_spi - spi driver
 * @param p_data_out - data to be written to the spi bus
 */
inline void write(spi& p_spi, std::span<const hal::byte> p_data_out)
{
  return p_spi.transfer(
    p_data_out, std::span<hal::byte>{}, spi::default_filler);
}

/**
 * @brief Read data from the spi bus.
 *
 * Filler bytes will be placed on the write/transmit line.
 *
 * @param p_spi - spi driver
 * @param p_data_in - buffer to receive bytes back from the spi bus
 * @param p_filler - filler data placed on the bus in place of actual data.
 */
inline void read(spi& p_spi,
                 std::span<hal::byte> p_data_in,
                 hal::byte p_filler = spi::default_filler)
{
  return p_spi.transfer(std::span<hal::byte>{}, p_data_in, p_filler);
}
/**
 * @brief Read data from the spi bus and return an array of bytes.
 *
 * Filler bytes will be placed on the write line.
 *
 * @tparam bytes_to_read - Number of bytes to read
 * @param p_spi - spi driver
 * @param p_filler - filler data placed on the bus in place of actual write
 * data.
 * @return std::array<hal::byte, bytes_to_read> - array containing bytes read
 * from the spi bus
 */
template<size_t bytes_to_read>
[[nodiscard]] std::array<hal::byte, bytes_to_read> read(
  spi& p_spi,
  hal::byte p_filler = spi::default_filler)
{
  std::array<hal::byte, bytes_to_read> buffer;
  p_spi.transfer(std::span<hal::byte>{}, buffer, p_filler);
  return buffer;
}

/**
 * @brief Perform a write-then-read operation over spi
 *
 * This utility function that fits the use case of many spi devices where a spi
 * transfer is not full duplex. In many spi devices, full duplex means that
 * as data is being written to the spi bus, the peripheral device is sending
 * data back on the receive line. In most cases, the device's communication
 * protocol is simply:
 *
 *    1. Write data to the bus, ignore the receive line
 *    2. Read from spi bus, filling the write line with filler
 *
 * @param p_spi - spi driver
 * @param p_data_out - bytes to write to the bus
 * @param p_data_in - buffer to receive bytes back from the spi bus
 * @param p_filler - filler data placed on the bus when the read operation
 * begins.
 */
inline void write_then_read(spi& p_spi,
                            std::span<const hal::byte> p_data_out,
                            std::span<hal::byte> p_data_in,
                            hal::byte p_filler = spi::default_filler)
{
  write(p_spi, p_data_out);
  read(p_spi, p_data_in, p_filler);
}

/**
 * @brief Perform a write-then-read operation over spi returning an array
 *
 * See write_then_read() for details about this function.
 *
 * @tparam bytes_to_read - Number of bytes to read from the bus
 * @param p_spi - spi driver
 * @param p_data_out - bytes to write to the bus
 * @param p_filler - filler data placed on the bus when the read operation
 * begins.
 * @return std::array<hal::byte, bytes_to_read> - array containing the bytes
 * read from the spi bus.
 */
template<size_t bytes_to_read>
[[nodiscard]] std::array<hal::byte, bytes_to_read> write_then_read(
  spi& p_spi,
  std::span<const hal::byte> p_data_out,
  hal::byte p_filler = spi::default_filler)
{
  write(p_spi, p_data_out);
  return read<bytes_to_read>(p_spi, p_filler);
}
}  // namespace hal
