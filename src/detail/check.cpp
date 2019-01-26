#include <unet/detail/check.hpp>

#include <cstring>

#include <boost/assert.hpp>

#include <unet/wire/wire.hpp>

namespace unet {
namespace detail {

std::uint16_t checksum(const std::uint8_t* buf, std::size_t bufLen) {
  BOOST_ASSERT(buf != nullptr);

  std::uint32_t acc = 0;
  auto end = buf + bufLen;
  auto p = buf;

  for (; p + 1 < end; p += 2) {
    std::uint16_t x;
    std::memcpy(&x, p, 2);
    acc += netToHost(x);
  }

  if (p < end) {
    acc += (static_cast<std::uint16_t>(*p) << 8);
  }

  while (acc > 0xffff) {
    acc -= 0xffff;
  }

  return hostToNet(static_cast<std::uint16_t>(~acc));
}

}  // namespace detail
}  // namespace unet
