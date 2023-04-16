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

#include <concepts>
#include <cstddef>
#include <limits>

namespace hal {
/**
 * @brief Generate a mask of 1s at compile time
 *
 * @tparam BitWidth - number of 1s in the mask
 * @tparam T - the type
 * @return consteval uint32_t - mask with 1s at the LSB
 */
template<size_t BitWidth, std::integral T>
[[nodiscard]] consteval T generate_field_of_ones()
{
  T result = 0;
  for (size_t i = 0; i < BitWidth; i++) {
    result |= 1 << i;
  }
  return result;
}

/**
 * @brief Similar to std::numeric_limits<T> except that this object can give
 * properties of integral types of arbitrary bit widths.
 *
 * @tparam BitWidth - bit width of the value
 * @tparam int_t - the containing type. Will generate a compile error if the
 * containing type's bit width is smaller than the BitWidth value.
 */
template<size_t BitWidth, std::integral int_t>
struct bit_limits
{
  // Check that the bit width is less than or equal to the size of the int_t.
  static_assert(BitWidth <= sizeof(int_t) * 8,
                "The BitWidth exceed the number of bits in the integer type.");

  // Check that bit width is not zero.
  static_assert(BitWidth != 0, "The BitWidth cannot be 0.");

  /**
   * @brief Get the maximum value available for an integer of `BitWidth` size
   * and that can be stored within `int_t`.
   *
   * The final value of the function depends also on the sign of the int type.
   *
   * @return constexpr int_t maximum value
   */
  [[nodiscard]] static constexpr int_t max()
  {
    if constexpr (std::is_signed_v<int_t>) {
      int_t max = generate_field_of_ones<BitWidth, int_t>();
      max = max >> 1;
      return max;
    } else {
      int_t max = generate_field_of_ones<BitWidth, int_t>();
      return max;
    }
  }

  /**
   * @brief Get the minimum value available for an integer of `BitWidth` size
   * and that can be stored within `int_t`.
   *
   * The final value of the function depends also on the sign of the int type.
   * Unsigned ints simply return zero.
   *
   * @return constexpr int_t minimum value
   */
  [[nodiscard]] static constexpr int_t min()
  {
    if constexpr (BitWidth == 64) {
      return std::numeric_limits<int_t>::min();
    } else if constexpr (std::is_signed_v<int_t>) {
      int_t min = (1ULL << BitWidth);
      return min;
    } else {
      return 0U;
    }
  }
};
}  // namespace hal
