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
#include <type_traits>

#include <libhal/error.hpp>
#include <libhal/timeout.hpp>
#include <libhal/units.hpp>

namespace hal {
namespace stream {
/**
 * @brief Discard received bytes until the sequence is found
 *
 */
class find
{
public:
  /**
   * @brief Construct a new find object
   *
   * @param p_sequence - sequence to search for. The lifetime of this data
   * pointed to by this span must outlive this object, or not be used when the
   * lifetime of that data is no longer available.
   */
  explicit find(std::span<const hal::byte> p_sequence);

  friend std::span<const hal::byte> operator|(
    const std::span<const hal::byte>& p_input_data,
    find& p_self);

  work_state state();

private:
  std::span<const hal::byte> m_sequence;
  size_t m_search_index = 0;
};

/**
 * @brief Non-blocking callable for reading serial data into a buffer
 *
 */
class fill
{
public:
  /**
   * @brief Construct a new fill object
   *
   * @param p_buffer - buffer to read data into
   */
  explicit fill(std::span<hal::byte> p_buffer);

  /**
   * @brief Construct a new fill object
   *
   * @param p_buffer - buffer to read data into
   * @param p_fill_amount - reference to a size value to limit the fill amount
   * by.
   */
  fill(std::span<hal::byte> p_buffer, const size_t& p_fill_amount);

  friend std::span<const hal::byte> operator|(
    const std::span<const hal::byte>& p_input_data,
    fill& p_self);

  work_state state();

private:
  std::span<hal::byte> m_buffer;
  const size_t* m_fill_amount = nullptr;
};

/**
 * @brief Discard received bytes until the sequence is found
 *
 */
class fill_upto
{
public:
  /**
   * @brief Construct a new fill upto object
   *
   * @param p_sequence - sequence to search for. The lifetime of this data
   * pointed to by this span must outlive this object, or not be used when the
   * lifetime of that data is no longer available.
   * @param p_buffer - buffer to fill data into
   */
  fill_upto(std::span<const hal::byte> p_sequence,
            std::span<hal::byte> p_buffer);

  friend std::span<const hal::byte> operator|(
    const std::span<const hal::byte>& p_input_data,
    fill_upto& p_self);

  work_state state();

  std::span<hal::byte> span();
  std::span<hal::byte> unfilled();

private:
  std::span<const hal::byte> m_sequence;
  std::span<hal::byte> m_buffer;
  size_t m_fill_amount = 0;
  size_t m_search_index = 0;
};

/**
 * @brief Read bytes from stream and convert to integer
 *
 */
template<std::unsigned_integral T>
class parse
{
public:
  /**
   * @brief Construct a new parse object
   */
  explicit parse() = default;

  friend std::span<const hal::byte> operator|(
    const std::span<const hal::byte>& p_input_data,
    parse& p_self)
  {
    if (p_self.m_finished) {
      return p_input_data;
    }

    for (size_t index = 0; index < p_input_data.size(); index++) {
      if (std::isdigit(static_cast<char>(p_input_data[index]))) {
        p_self.m_value *= 10;
        p_self.m_value += p_input_data[index] - hal::byte('0');
        p_self.m_found_digit = true;
      } else if (p_self.m_found_digit) {
        p_self.m_finished = true;
        return p_input_data.subspan(index);
      }
    }

    return p_input_data.last(0);
  }

  work_state state()
  {
    return m_finished ? work_state::finished : work_state::in_progress;
  }

  /**
   * @return T& - return an immutable reference to the value
   */
  const T& value()
  {
    return m_value;
  }

private:
  T m_value = 0;
  bool m_found_digit = false;
  bool m_finished = false;
};

/**
 * @brief Skip number of bytes in a byte stream
 *
 */
class skip
{
public:
  /**
   * @brief Construct a new skip object
   *
   */
  explicit skip(size_t p_skip);

  friend std::span<const hal::byte> operator|(
    const std::span<const hal::byte>& p_input_data,
    skip& p_self);

  work_state state();

private:
  size_t m_skip;
};
}  // namespace stream
}  // namespace hal
