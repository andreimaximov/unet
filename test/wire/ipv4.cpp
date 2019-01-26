#include <gmock/gmock.h>
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

TEST(Ipv4Test, Ipv4AddrCidr) {
  Ipv4Addr addr1{10, 255, 255, 101};
  Ipv4Addr addr2{10, 255, 255, 102};
  Ipv4Addr addr3{10, 255, 254, 101};

  Ipv4AddrCidr addrCidr{addr1, 24};

  ASSERT_TRUE(addrCidr.isInSubnet(addr1));
  ASSERT_TRUE(addrCidr.isInSubnet(addr2));
  ASSERT_FALSE(addrCidr.isInSubnet(addr3));
}

TEST(Ipv4Test, PackedHeader) {
  std::array<std::uint8_t, sizeof(Ipv4Header)> buf{};

  auto ipv4 = reinterpret_cast<Ipv4Header*>(buf.data());
  ipv4->version = 4;
  ipv4->ihl = 5;
  ipv4->dscp = 5;
  ipv4->ecn = 2;
  ipv4->len = hostToNet<std::uint16_t>(20);
  ipv4->id = hostToNet<std::uint16_t>(1);
  ipv4->flagsOffset = 0;
  ipv4->ttl = 64;
  ipv4->proto = ipv4_proto::kIcmp;
  ipv4->checksum = 0;
  ipv4->srcAddr = Ipv4Addr{10, 255, 255, 102};
  ipv4->dstAddr = Ipv4Addr{10, 255, 255, 101};
  ipv4->checksum = checksumIpv4(ipv4);

  ASSERT_THAT(
      buf, testing::ElementsAre(0x45, 0x16, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00,
                                0x40, 0x01, 0x66, 0x08, 0x0A, 0xFF, 0xFF, 0x66,
                                0x0A, 0xFF, 0xFF, 0x65));
}

}  // namespace unet
