#pragma once

#include <cstddef>
#include <cstdint>

#include <unet/wire/wire.hpp>

namespace unet {

struct UNET_PACK Icmpv4Header {
  std::uint8_t type;
  std::uint8_t code;
  std::uint16_t checksum;
  std::uint8_t data[4];
};

UNET_ASSERT_SIZE(Icmpv4Header, 8);

struct UNET_PACK Icmpv4Echo {
  std::uint16_t id;
  std::uint16_t seq;
};

UNET_ASSERT_SIZE(Icmpv4Echo, 4);

// Return an ICMPv4 header checksum.
std::uint16_t checksumIcmpv4(const Icmpv4Header* header,
                             std::size_t payloadLen);

}  // namespace unet
