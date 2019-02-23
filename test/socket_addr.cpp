#include <gtest/gtest.h>

#include <unet/socket_addr.hpp>

namespace unet {

TEST(SocketAddrTest, Eq) {
  Ipv4Addr a{10, 255, 255, 1};
  Ipv4Addr b{10, 255, 255, 2};

  ASSERT_EQ(SocketAddr(a, 1), SocketAddr(a, 1));
  ASSERT_NE(SocketAddr(a, 1), SocketAddr(a, 2));
  ASSERT_NE(SocketAddr(a, 1), SocketAddr(b, 1));
  ASSERT_NE(SocketAddr(a, 1), SocketAddr(b, 2));
}

}  // namespace unet
