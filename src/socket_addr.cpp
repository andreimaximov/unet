#include <unet/socket_addr.hpp>

namespace unet {

SocketAddr::SocketAddr(Ipv4Addr addr, std::uint16_t port)
    : addr_{addr}, port_{port} {}

Ipv4Addr SocketAddr::addr() const {
  return addr_;
}

std::uint16_t SocketAddr::port() const {
  return port_;
}

bool SocketAddr::operator==(const SocketAddr& addr) const {
  return addr_ == addr.addr_ && port_ == addr.port_;
}

bool SocketAddr::operator!=(const SocketAddr& addr) const {
  return !(*this == addr);
}

}  // namespace unet
