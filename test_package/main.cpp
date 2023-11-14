// Copyright 2023 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <cstdio>

#include <libhal-util/to_array.hpp>

int main()
{
  using namespace std::string_view_literals;

  auto array = hal::to_array<5>("Hello World"sv);

  std::printf("array = { ");
  for (const auto& byte : array) {
    std::printf("0x%02X ", byte);
  }

  std::printf("};\n");

  return 0;
}
