#pragma once

#include <cstdint>
#include <ostream>

#include <unet/wire/wire.hpp>

namespace unet {

struct UNET_PACK EthernetAddr {
  struct MemcmpTag {};
  std::uint8_t addr[6];
};

UNET_ASSERT_SIZE(EthernetAddr, 6);

std::ostream& operator<<(std::ostream& os, EthernetAddr ethAddr);

struct UNET_PACK EthernetHeader {
  struct MemcmpTag {};
  EthernetAddr dstAddr;
  EthernetAddr srcAddr;
  std::uint16_t ethType;
};

UNET_ASSERT_SIZE(EthernetHeader, 14);

constexpr EthernetAddr kEthernetBcastAddr{{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};

// EtherType constants: https://en.wikipedia.org/wiki/EtherType
namespace eth_type {
static const std::uint16_t kIpv4 = hostToNet<std::uint16_t>(0x0800);
static const std::uint16_t kArp = hostToNet<std::uint16_t>(0x0806);
}  // namespace eth_type

}  // namespace unet

UNET_STD_HASH_AS_MEM_HASH(unet::EthernetAddr)
UNET_STD_HASH_AS_MEM_HASH(unet::EthernetHeader)
