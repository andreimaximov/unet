#pragma once

#include <chrono>
#include <cstddef>
#include <memory>
#include <unordered_map>

#include <boost/optional.hpp>

#include <unet/detail/arp_cache.hpp>
#include <unet/detail/frame.hpp>
#include <unet/detail/nonmovable.hpp>
#include <unet/detail/queue.hpp>
#include <unet/timer.hpp>
#include <unet/wire/ethernet.hpp>
#include <unet/wire/ipv4.hpp>
#include <unet/wire/wire.hpp>

namespace unet {
namespace detail {

// A queue for delaying IPv4 frames w/an unresolved IPv4 address.
class ArpQueue : private NonMovable {
 public:
  ArpQueue(std::size_t delayQueueLen, std::size_t cacheCapacity,
           std::chrono::seconds delayTimeout, std::chrono::seconds cacheTTL,
           std::shared_ptr<Queue> sendQueue,
           std::shared_ptr<TimerManager> timerManager);

  // Adds an IPv4 -> Ethernet address mapping to the underlying cache and sends
  // any delayed IPv4 frames.
  void add(Ipv4Addr hopAddr, EthernetAddr ethAddr);

  // Return an Ethernet address mapping for an IPv4 address.
  boost::optional<EthernetAddr> lookup(Ipv4Addr hopAddr);

  // Delays transmission of an IPv4 frame until an Ethernet address for the next
  // hop IPv4 address is resolved. The frame might be dropped due to an ARP
  // timeout or queue capacity limitations.
  void delay(std::unique_ptr<Frame> frame);

 private:
  void scheduleTimeout(Ipv4Addr hopAddr);

  Queue delayQueue_;
  std::chrono::seconds delayTimeout_;
  ArpCache cache_;
  std::shared_ptr<Queue> sendQueue_;
  std::shared_ptr<TimerManager> timerManager_;
  std::unordered_map<Ipv4Addr, std::unique_ptr<Timer>, Memhash<Ipv4Addr>>
      timers_;
};

}  // namespace detail
}  // namespace unet
