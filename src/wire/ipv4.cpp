#include <unet/wire/ipv4.hpp>

#include <regex>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/numeric/conversion/cast.hpp>

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

Ipv4Addr parseIpv4(boost::string_view rawIpv4Addr) {
  static const std::regex kIpv4Base{
      "\\s*([0-9]+)\\.([0-9]+)\\.([0-9]+)\\.([0-9]+)\\s*"};

  std::match_results<boost::string_view::const_iterator> tokens;
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

}  // namespace unet
