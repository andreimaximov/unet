#include <unet/wire/ipv4.hpp>

#include <cstring>
#include <regex>

#include <boost/assert.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/numeric/conversion/cast.hpp>

#include <unet/detail/check.hpp>
#include <unet/exception.hpp>

namespace unet {

constexpr auto kBadIpv4Addr = "Cannot parse bad IPv4 address.";

template <typename T>
static std::uint8_t asUnsignedInt8(const T& src) {
  return boost::numeric_cast<std::uint8_t>(
      boost::lexical_cast<std::uint32_t>(src));
}

std::ostream& operator<<(std::ostream& os, Ipv4Addr ipv4Addr) {
  boost::format fmt{"%d.%d.%d.%d"};
  for (auto p = ipv4Addr.addr; p < ipv4Addr.addr + 4; p++) {
    fmt % static_cast<std::uint32_t>(*p);
  }
  return os << fmt;
}

Ipv4Addr parseIpv4(const std::string& rawIpv4Addr) {
  static const std::regex kIpv4Base{
      "\\s*([0-9]+)\\.([0-9]+)\\.([0-9]+)\\.([0-9]+)\\s*"};

  std::smatch tokens;
  if (!std::regex_match(rawIpv4Addr.begin(), rawIpv4Addr.end(), tokens,
                        kIpv4Base)) {
    throw Exception{kBadIpv4Addr};
  }

  Ipv4Addr addr;

  try {
    addr.addr[0] = asUnsignedInt8(tokens[1]);
    addr.addr[1] = asUnsignedInt8(tokens[2]);
    addr.addr[2] = asUnsignedInt8(tokens[3]);
    addr.addr[3] = asUnsignedInt8(tokens[4]);
  } catch (const std::exception&) {
    throw Exception{kBadIpv4Addr};
  }

  return addr;
}

Ipv4AddrCidr::Ipv4AddrCidr(Ipv4Addr addr, std::size_t maskLen) : addr_{addr} {
  BOOST_ASSERT(maskLen <= 32);
  mask_ = hostToNet((~mask_) << (32 - maskLen));
}

Ipv4Addr Ipv4AddrCidr::operator*() const {
  return addr_;
}

bool Ipv4AddrCidr::isInSubnet(Ipv4Addr addr) const {
  auto addrAsInt = [](Ipv4Addr addr) {
    std::uint32_t raw;
    std::memcpy(&raw, addr.addr, 4);
    return raw;
  };

  return (addrAsInt(addr_) & mask_) == (addrAsInt(addr) & mask_);
}

std::uint16_t checksumIpv4(const Ipv4Header* header) {
  return detail::checksum(reinterpret_cast<const std::uint8_t*>(header),
                          sizeof(Ipv4Header));
}

}  // namespace unet
