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

#include <libhal-util/bit.hpp>

#include <boost/ut.hpp>

namespace hal {
void bit_test()
{
  using namespace boost::ut;

  "hal::bit<template> standard usage"_test = []() {
    // Setup
    volatile std::uint32_t control_register = 1 << 15 | 1 << 16;
    constexpr auto enable_bit = bitmask::from<1>();
    constexpr auto high_power_mode = bitmask::from<15>();
    constexpr auto clock_divider = bitmask::from<20, 23>();
    constexpr auto extractor_mask = bitmask::from<16, 23>();
    constexpr auto single_bitmask = bitmask::from<1>();

    // Exercise
    modify(control_register)
      .set<enable_bit>()
      .clear<high_power_mode>()
      .insert<clock_divider>(0xAU);
    auto extracted = extract<extractor_mask>(control_register);
    auto probed = extract<single_bitmask>(control_register);
    auto probed_inline =
      extract<bitmask{ .position = 15, .width = 1 }>(control_register);

    // Verify
    expect(that % 0x00A1'0002 == control_register);
    expect(that % 0xA1 == extracted);
    expect(that % 1 == probed);
    expect(that % 0 == probed_inline);
  };

  "hal::bit standard usage "_test = []() {
    // Setup
    volatile std::uint32_t control_register = 1 << 15 | 1 << 16;
    constexpr auto enable_bit = bitmask::from<1>();
    constexpr auto high_power_mode = bitmask::from<15>();
    constexpr auto clock_divider = bitmask::from<20, 23>();
    constexpr auto extractor_mask = bitmask::from<16, 23>();
    constexpr auto single_bitmask = bitmask::from<1>();

    // Exercise
    modify(control_register)
      .set(enable_bit)
      .clear(high_power_mode)
      .insert(clock_divider, 0xAU);
    auto extracted = extract(extractor_mask, control_register);
    auto probed = extract(single_bitmask, control_register);
    auto probed_inline =
      extract(bitmask{ .position = 15, .width = 1 }, control_register);

    // Verify
    expect(that % 0x00A1'0002 == control_register);
    expect(that % 0xA1 == extracted);
    expect(that % 1 == probed);
    expect(that % 0 == probed_inline);
  };
};
}  // namespace hal
