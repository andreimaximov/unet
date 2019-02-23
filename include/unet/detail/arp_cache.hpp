#pragma once

#include <chrono>
#include <cstddef>
#include <functional>
#include <unordered_map>

#include <boost/optional.hpp>

#include <unet/wire/ethernet.hpp>
#include <unet/wire/ipv4.hpp>
#include <unet/wire/wire.hpp>

namespace unet {
namespace detail {

// A cache for IPv4 -> Ethernet address mappings.
class ArpCache {
 public:
  // Creates an ARP cache w/the specified capacity and expiration time (TTL)
  // for each mapping. Garbage collection will run once the cache exceeds its
  // capacity. Expired entries are evicted before least recently used ones.
  ArpCache(std::size_t capacity, std::chrono::seconds ttl,
           std::function<std::chrono::steady_clock::time_point()> now =
               std::chrono::steady_clock::now);

  // Adds an Ipv4 -> Ethernet mapping to the cache.
  void add(Ipv4Addr ipv4Addr, EthernetAddr ethAddr);

  // Return an Ethernet address mapping for an Ipv4 address.
  boost::optional<EthernetAddr> lookup(Ipv4Addr ipv4Addr);

 private:
  struct Item {
    EthernetAddr addr;
    std::chrono::steady_clock::time_point usedAt;
    std::chrono::steady_clock::time_point expireAt;
  };

  void gc(std::chrono::steady_clock::time_point now);

  std::unordered_map<Ipv4Addr, Item> cache_;
  std::size_t capacity_;
  std::chrono::seconds ttl_;
  std::function<std::chrono::steady_clock::time_point()> now_;
};

}  // namespace detail
}  // namespace unet
