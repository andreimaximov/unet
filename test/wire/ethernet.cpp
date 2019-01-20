#include <gtest/gtest.h>

#include <boost/format.hpp>

#include <unet/wire/ethernet.hpp>

namespace unet {

TEST(EthernetTest, OutputStream) {
  EthernetAddr addr{0xA1, 0xB2, 0xC3, 0xD4, 0xE5, 0xF6};
  auto asStr = boost::str(boost::format("%1%") % addr);
  ASSERT_EQ(asStr, "A1:B2:C3:D4:E5:F6");
}

}  // namespace unet
