#pragma once

#include <functional>
#include <memory>

#include <unet/detail/list.hpp>
#include <unet/detail/nonmovable.hpp>
#include <unet/detail/queue.hpp>
#include <unet/detail/raw_socket.hpp>
#include <unet/detail/socket_set.hpp>
#include <unet/dev/dev.hpp>
#include <unet/options.hpp>
#include <unet/timer.hpp>

namespace unet {

// The core of the network stack. Responsible for draining sockets, routing
// packets, etc.
class Stack : public detail::NonMovable {
 public:
  // Creates a network stack powered by the provided device.
  Stack(std::unique_ptr<Dev> dev, Options opts = Options{});

  // Run the network stack until stopLoop(...) is called or an error occurs.
  void runLoop();

  // Stops the network stack loop. Does nothing if the network stack is not
  // running.
  void stopLoop();

  // Return a timer which will run f upon expiration.
  std::unique_ptr<Timer> createTimer(std::function<void()> f);

 private:
  void runLoopOnce();

  void sendLoop();

  void readLoop();

  std::unique_ptr<Dev> dev_;
  Options opts_;
  detail::SocketSet socketSet_;
  detail::Queue sendQueue_;
  detail::List<detail::RawSocket> ethernetSockets_;
  TimerManager timerManager_;
  bool runningLoop_ = false;

  friend class RawSocket;
};

}  // namespace unet
