#pragma once

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

}  // namespace unet
