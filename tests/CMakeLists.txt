cmake_minimum_required(VERSION 3.15)

project(unit_test VERSION 0.0.1 LANGUAGES CXX)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  # require at least gcc 11
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 11.0)
    message(FATAL_ERROR "GCC version must be at least 11!")
  endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  # require at least clang 14
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 14)
    message(FATAL_ERROR "Clang version must be at least 14!")
  endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
  # require at least clang 14
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 14)
    message(FATAL_ERROR "Clang version must be at least 14!")
  endif()
else()
  message(WARNING "You are using an unsupported compiler! Compilation has only been tested with Clang and GCC, detected ${CMAKE_CXX_COMPILER_ID}.")
endif()

list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})
set(CMAKE_BUILD_TYPE "Debug")

find_package(ut REQUIRED CONFIG)
find_package(libhal REQUIRED CONFIG)

add_executable(${PROJECT_NAME}
  as_bytes.test.cpp
  bit.test.cpp
  can.test.cpp
  enum.test.cpp
  i2c.test.cpp
  input_pin.test.cpp
  interrupt_pin.test.cpp
  map.test.cpp
  math.test.cpp
  move_interceptor.test.cpp
  output_pin.test.cpp
  overflow_counter.test.cpp
  serial.test.cpp
  spi.test.cpp
  static_callable.test.cpp
  static_list.test.cpp
  steady_clock.test.cpp
  streams.test.cpp
  timeout.test.cpp
  units.test.cpp

  main.test.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC . ../include)
target_compile_options(${PROJECT_NAME} PRIVATE
  --coverage
  -fprofile-arcs
  -ftest-coverage
  -Werror
  -Wall
  -Wextra
  -Wshadow
  -Wnon-virtual-dtor
  -Wno-gnu-statement-expression
  -pedantic
  -Wno-unneeded-internal-declaration)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)
target_link_options(${PROJECT_NAME} PRIVATE
  --coverage
  -fprofile-arcs
  -ftest-coverage)
target_link_libraries(${PROJECT_NAME} PRIVATE
  Boost::ut
  libhal::libhal)
