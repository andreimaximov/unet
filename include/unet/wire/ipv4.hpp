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

}  // namespace unet
