#include <libhal-util/enum.hpp>

#include <boost/ut.hpp>

namespace hal {
void enum_test()
{
  using namespace boost::ut;

  "hal::value(enum)"_test = []() {
    constexpr int expected1 = 1;
    constexpr int expected2 = 20;

    enum class test_enum
    {
      enum_value1 = expected1,
      enum_value2 = expected2,
    };

    expect(that % expected1 == value(test_enum::enum_value1));
    expect(that % expected2 == value(test_enum::enum_value2));
  };
};
}  // namespace hal
