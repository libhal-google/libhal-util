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

#include <libhal/output_pin.hpp>

namespace hal {
[[nodiscard]] constexpr auto operator==(const output_pin::settings& p_lhs,
                                        const output_pin::settings& p_rhs)
{
  return p_lhs.resistor == p_rhs.resistor &&
         p_lhs.open_drain == p_rhs.open_drain;
}
}  // namespace hal
