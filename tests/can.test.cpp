#include <libhal-util/can.hpp>

#include <boost/ut.hpp>

namespace hal {
void can_test()
{
  using namespace boost::ut;

  "operator==(can::message, can::message) "_test = []() {
    can::message_t a = {
      .id = 0x111,
      .payload = { 0xAA },
      .length = 1,
      .is_remote_request = false,
    };

    can::message_t b = {
      .id = 0x111,
      .payload = { 0xAA },
      .length = 1,
      .is_remote_request = false,
    };

    can::message_t c = {
      .id = 0x112,
      .payload = { 0xAB },
      .length = 1,
      .is_remote_request = false,
    };

    expect(a == b);
    expect(a != c);
    expect(b != c);
  };
}
}  // namespace hal