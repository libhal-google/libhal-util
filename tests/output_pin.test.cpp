#include <libhal-util/output_pin.hpp>

#include <boost/ut.hpp>

namespace hal {
boost::ut::suite output_pin_util_test = []() {
  using namespace boost::ut;
  "operator==(output_pin::settings)"_test = []() {
    output_pin::settings a{};
    output_pin::settings b{};

    expect(a == b);
  };

  "operator!=(output_pin::settings)"_test = []() {
    output_pin::settings a{ .open_drain = true };
    output_pin::settings b{ .open_drain = false };

    expect(a != b);
  };
};
}  // namespace hal
