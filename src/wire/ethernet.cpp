#include <unet/wire/ethernet.hpp>

#include <boost/format.hpp>

namespace unet {

std::ostream& operator<<(std::ostream& os, EthernetAddr ethAddr) {
  boost::format fmt{"%02X:%02X:%02X:%02X:%02X:%02X"};
  for (auto p = ethAddr.addr; p < ethAddr.addr + 6; p++) {
    fmt % static_cast<std::uint32_t>(*p);
  }
  return os << fmt;
}

}  // namespace unet
