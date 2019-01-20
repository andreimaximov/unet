#pragma once

#include <cstring>

#include <boost/endian/conversion.hpp>

namespace unet {

#define UNET_PACK __attribute__((packed))

#define UNET_ASSERT_SIZE(type, size) \
  static_assert(sizeof(type) == size, "Type has an unexpected size!");

// Performs a host to network byte order conversion on x.
//
// Return x in network byte order
template <typename T>
T hostToNet(T x) {
  return boost::endian::endian_reverse(x);
}

// Performs a network to host byte order conversion on x.
//
// Return x in host byte order.
template <typename T>
T netToHost(T x) {
  return hostToNet(x);
}

// A std::memcmp based == operator for types which declare a MemcmpTag.
template <typename T, typename = typename T::MemcmpTag>
bool operator==(const T& lhs, const T& rhs) {
  return std::memcmp(&lhs, &rhs, sizeof(T)) == 0;
}

// A std::memcmp based != operator for types which declare a MemcmpTag.
template <typename T, typename = typename T::MemcmpTag>
bool operator!=(const T& lhs, const T& rhs) {
  return !(lhs == rhs);
}

}  // namespace unet
