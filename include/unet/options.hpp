#pragma once

#include <cstddef>

namespace unet {

struct Options {
  // The maximum number of egress frames the stack can queue before tail (1)
  // tail dropping or (2) delaying dispatch of socket frames. This does not
  // count frames queued due for IPv4 -> Ethernet address resolution via ARP.
  std::size_t stackSendQueueLen = 16'384;

  // The maximum number of frames a raw socket can queue on the send path.
  std::size_t rawSocketSendQueueLen = 1024;

  // The maximum number of frames a raw socket can queue on the read path.
  std::size_t rawSocketReadQueueLen = 1024;
};

}  // namespace unet
