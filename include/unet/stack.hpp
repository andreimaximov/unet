#pragma once

#include <cstdint>
#include <functional>
#include <memory>

#include <unet/detail/arp_queue.hpp>
#include <unet/detail/frame.hpp>
#include <unet/detail/list.hpp>
#include <unet/detail/nonmovable.hpp>
#include <unet/detail/queue.hpp>
#include <unet/detail/raw_socket.hpp>
#include <unet/detail/socket_set.hpp>
#include <unet/dev/dev.hpp>
#include <unet/options.hpp>
#include <unet/timer.hpp>
#include <unet/wire/ethernet.hpp>
#include <unet/wire/ipv4.hpp>

namespace unet {

// The core of the network stack. Responsible for draining sockets, routing
// packets, etc.
class Stack : public detail::NonMovable {
 public:
  // Creates a network stack powered by the provided device. The stack is
  // assigned the specified Ethernet and IPv4 addresses.
  Stack(std::unique_ptr<Dev> dev, EthernetAddr ethAddr,
        Ipv4AddrCidr ipv4AddrCidr, Ipv4Addr defaultGateway,
        Options opts = Options{});

  // Run the network stack until stopLoop(...) is called or an error occurs.
  void runLoop();

  // Stops the network stack loop. Does nothing if the network stack is not
  // running.
  void stopLoop();

  // Return a timer which will run f upon expiration.
  std::unique_ptr<Timer> createTimer(std::function<void()> f);

  // Return the Ethernet address assigned to the stack.
  EthernetAddr getHwAddr() const;

  // Return the IPv4 address assigned to the stack.
  Ipv4Addr getIpv4Addr() const;

 private:
  void runLoopOnce();
  void sendLoop();
  void readLoop();
  void process(detail::Frame& f);
  void processArp(detail::Frame& f);
  void sendArp(Ipv4Addr dstIpv4Addr, EthernetAddr dstHwAddr,
               std::uint16_t arpOp);
  void processIpv4(detail::Frame& f);
  bool sendIpv4(detail::Frame& f);

  std::unique_ptr<Dev> dev_;
  EthernetAddr ethAddr_;
  Ipv4AddrCidr ipv4AddrCidr_;
  Ipv4Addr defaultGateway_;
  Options opts_;
  detail::SocketSet socketSet_;
  std::shared_ptr<detail::Queue> sendQueue_;
  detail::List<detail::RawSocket> ethernetSockets_;
  detail::List<detail::RawSocket> ipv4Sockets_;
  std::shared_ptr<TimerManager> timerManager_;
  detail::ArpQueue arpQueue_;
  bool runningLoop_ = false;

  friend class RawSocket;
};

}  // namespace unet
