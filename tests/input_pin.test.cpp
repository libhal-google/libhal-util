#include <libhal-util/input_pin.hpp>

#include <boost/ut.hpp>

namespace hal {
boost::ut::suite input_pin_util_test = []() {
  using namespace boost::ut;
  "operator==(input_pin::settings)"_test = []() {
    input_pin::settings a{};
    input_pin::settings b{};

    expect(a == b);
  };

  "operator!=(input_pin::settings)"_test = []() {
    input_pin::settings a{ .resistor = pin_resistor::pull_up };
    input_pin::settings b{ .resistor = pin_resistor::none };

    expect(a != b);
  };
};
}  // namespace hal
