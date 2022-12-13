#include <libhal-util/interrupt_pin.hpp>

#include <boost/ut.hpp>

namespace hal {
void interrupt_pin_util_test()
{
  using namespace boost::ut;
  "operator==(interrupt_pin::settings)"_test = []() {
    interrupt_pin::settings a{};
    interrupt_pin::settings b{};

    expect(a == b);
  };

  "operator!=(interrupt_pin::settings)"_test = []() {
    interrupt_pin::settings a{ .resistor = pin_resistor::pull_up };
    interrupt_pin::settings b{ .resistor = pin_resistor::none };

    expect(a != b);
  };
};
}  // namespace hal
