#pragma once

#include <cstddef>
#include <cstdint>

#include <unet/wire/ipv4.hpp>

namespace unet {

// A socket endpoint identified by an IPv4 address and a TCP/UDP port.
class SocketAddr {
 public:
  SocketAddr(Ipv4Addr addr, std::uint16_t port);

  SocketAddr(const SocketAddr&) = default;
  SocketAddr(SocketAddr&&) = default;
  SocketAddr& operator=(const SocketAddr&) = default;
  SocketAddr& operator=(SocketAddr&&) = default;

  Ipv4Addr addr() const;

  std::uint16_t port() const;

  bool operator==(const SocketAddr& addr) const;
  bool operator!=(const SocketAddr& addr) const;

 private:
  Ipv4Addr addr_;
  std::uint16_t port_;
};

}  // namespace unet

UNET_STD_HASH_AS_MEM_HASH(unet::SocketAddr)
