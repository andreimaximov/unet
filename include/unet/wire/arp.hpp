#pragma once

#include <cstdint>

#include <unet/wire/ethernet.hpp>
#include <unet/wire/ipv4.hpp>
#include <unet/wire/wire.hpp>

namespace unet {

struct UNET_PACK ArpHeader {
  struct MemcmpTag {};
  std::uint16_t hwType;
  std::uint16_t protoType;
  std::uint8_t hwLen;
  std::uint8_t protoLen;
  std::uint16_t op;
  EthernetAddr srcHwAddr;
  Ipv4Addr srcProtoAddr;
  EthernetAddr dstHwAddr;
  Ipv4Addr dstProtoAddr;
};

UNET_ASSERT_SIZE(ArpHeader, 28);

namespace arp_op {
static const std::uint16_t kRequest = hostToNet<std::uint16_t>(0x0001);
static const std::uint16_t kReply = hostToNet<std::uint16_t>(0x0002);
}  // namespace arp_op

namespace arp_hw_addr {
static const std::uint16_t kEth = hostToNet<std::uint16_t>(0x0001);
}  // namespace arp_hw_addr

namespace arp_proto_addr {
static const std::uint16_t kIpv4 = hostToNet<std::uint16_t>(0x0800);
}  // namespace arp_proto_addr

}  // namespace unet
