#pragma once

#include <cstddef>
#include <cstdint>
#include <ostream>

#include <boost/utility/string_view.hpp>

#include <unet/wire/wire.hpp>

namespace unet {

struct UNET_PACK Ipv4Addr {
  struct MemcmpTag {};
  std::uint8_t addr[4];
};

UNET_ASSERT_SIZE(Ipv4Addr, 4);

constexpr Ipv4Addr kLoopback{{127, 0, 0, 1}};

std::ostream& operator<<(std::ostream& os, Ipv4Addr ipv4Addr);

// Return a parsed IPv4 address from a string. Throws an Exception in case of an
// error.
Ipv4Addr parseIpv4(boost::string_view rawIpv4Addr);

// An IPv4 address w/a subnet mask.
class Ipv4AddrCidr {
 public:
  Ipv4AddrCidr(Ipv4Addr addr, std::size_t maskLen);

  Ipv4Addr operator*() const;

  bool isInSubnet(Ipv4Addr addr) const;

 private:
  Ipv4Addr addr_{};
  std::uint32_t mask_ = 0;
};

struct UNET_PACK Ipv4Header {
// Careful with order of members on compilers for targets w/different endianess.
#ifdef BOOST_LITTLE_ENDIAN
  std::uint8_t ihl : 4;
  std::uint8_t version : 4;
  std::uint8_t ecn : 2;
  std::uint8_t dscp : 6;
#elif BOOST_BIG_ENDIAN
  std::uint8_t version : 4;
  std::uint8_t ihl : 4;
  std::uint8_t dscp : 6;
  std::uint8_t ecn : 2;
#endif
  std::uint16_t len;
  std::uint16_t id;
  std::uint16_t flagsOffset;
  std::uint8_t ttl;
  std::uint8_t proto;
  std::uint16_t checksum;
  Ipv4Addr srcAddr;
  Ipv4Addr dstAddr;
  std::uint8_t options[];
};

UNET_ASSERT_SIZE(Ipv4Header, 20);

// Return an IPv4 header checksum.
std::uint16_t checksumIpv4(const Ipv4Header* header);

// [IPv4
// protocols](https://www.iana.org/assignments/protocol-numbers/protocol-numbers.xhtml)
namespace ipv4_proto {
static const std::uint8_t kIcmp = 1;
}  // namespace ipv4_proto

}  // namespace unet
