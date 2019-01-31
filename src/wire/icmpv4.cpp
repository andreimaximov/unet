#include <unet/wire/icmpv4.hpp>

#include <unet/detail/check.hpp>

namespace unet {

std::uint16_t checksumIcmpv4(const Icmpv4Header* header,
                             std::size_t payloadLen) {
  return detail::checksum(reinterpret_cast<const std::uint8_t*>(header),
                          sizeof(Icmpv4Header) + payloadLen);
}

}  // namespace unet
