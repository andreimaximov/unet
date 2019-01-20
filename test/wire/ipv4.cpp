#include <gtest/gtest.h>

#include <boost/format.hpp>

#include <unet/exception.hpp>
#include <unet/wire/ipv4.hpp>

namespace unet {

TEST(Ipv4Test, OutputStream) {
  Ipv4Addr addr{10, 255, 255, 101};
  auto asStr = boost::str(boost::format("%1%") % addr);
  ASSERT_EQ(asStr, "10.255.255.101");
}

TEST(Ipv4Test, ParseIpv4Valid) {
  auto addr1 = parseIpv4("1.2.3.4");
  ASSERT_EQ(addr1, (Ipv4Addr{1, 2, 3, 4}));

  auto addr2 = parseIpv4("  1.2.3.4  ");
  ASSERT_EQ(addr2, (Ipv4Addr{1, 2, 3, 4}));
}

TEST(Ipv4Test, ParseIpv4Invalid) {
  ASSERT_THROW(parseIpv4(""), Exception);
  ASSERT_THROW(parseIpv4("1.2.3.999"), Exception);
  ASSERT_THROW(parseIpv4("1.2.3.4.5"), Exception);
  ASSERT_THROW(parseIpv4(" 1 . 2 . 3 . 4 "), Exception);
}

}  // namespace unet
