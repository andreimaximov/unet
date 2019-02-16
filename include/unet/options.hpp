#pragma once

#include <chrono>
#include <cstddef>

namespace unet {

struct Options {
  // The maximum number of egress frames the stack can queue before (1) tail
  // dropping or (2) delaying dispatch of socket frames. This does not count
  // frames queued due for IPv4 -> Ethernet address resolution via ARP.
  std::size_t stackSendQueueLen = 16'384;

  // The maximum number of frames waiting for an ARP reply that can be queued.
  std::size_t arpQueueLen = 1'024;

  // The maximum number of ARP entries to cache before evicting according to an
  // LRU policy.
  std::size_t arpCacheSize = 1'024;

  // The maximum time a frame waiting for an ARP reply is queued before being
  // dropped.
  std::chrono::seconds arpTimeout = std::chrono::seconds{1};

  // The maximum time an ARP entry is kept in cache before expiring.
  std::chrono::seconds arpCacheTTL = std::chrono::seconds{60};

  // The maximum number of bytes a raw socket can queue on the send path.
  std::size_t rawSocketSendQueueLen = 32'768;

  // The maximum number of bytes a raw socket can queue on the read path.
  std::size_t rawSocketReadQueueLen = 32'768;
};

}  // namespace unet
