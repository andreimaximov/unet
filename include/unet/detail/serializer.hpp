#pragma once

#include <cstddef>
#include <cstdint>

#include <unet/detail/frame.hpp>
#include <unet/wire/ethernet.hpp>
#include <unet/wire/ipv4.hpp>
#include <unet/wire/wire.hpp>

namespace unet {
namespace detail {

// A serializer for crafting frames based on payloads from different layers.
class Serializer {
 public:
  Serializer(EthernetAddr ethAddr, Ipv4Addr ipv4Addr);

  template <typename F>
  auto make(std::size_t netLen, F&& callback) {
    auto f = detail::Frame::makeUninitialized(sizeof(EthernetHeader) + netLen);
    f->dataAs<EthernetHeader>()->srcAddr = ethAddr_;
    f->net = f->data + sizeof(EthernetHeader);
    f->netLen = netLen;
    callback(*f);
    return f;
  }

  template <typename F>
  auto makeIpv4(std::size_t transportLen, F&& callback) {
    auto netLen = sizeof(Ipv4Header) + transportLen;
    return make(netLen,
                [this, &callback, transportLen, netLen](detail::Frame& f) {
                  auto eth = f.dataAs<EthernetHeader>();
                  eth->ethType = eth_type::kIpv4;

                  auto ipv4 = f.netAs<Ipv4Header>();
                  ipv4->ihl = 5;
                  ipv4->version = 4;
                  ipv4->ecn = 0;
                  ipv4->dscp = 0;
                  ipv4->len = hostToNet<std::uint16_t>(netLen);
                  ipv4->id = 0;
                  ipv4->flagsOffset = 0;
                  ipv4->ttl = 64;
                  ipv4->srcAddr = ipv4Addr_;
                  f.transport = f.net + sizeof(Ipv4Header);
                  f.transportLen = transportLen;
                  f.doIpv4Routing = true;

                  callback(f);

                  ipv4->checksum = 0;
                  ipv4->checksum = checksumIpv4(ipv4);
                });
  }

 private:
  EthernetAddr ethAddr_;
  Ipv4Addr ipv4Addr_;
};

}  // namespace detail
}  // namespace unet
