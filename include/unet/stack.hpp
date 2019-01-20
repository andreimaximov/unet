#pragma once

#include <memory>

#include <unet/detail/list.hpp>
#include <unet/detail/nonmovable.hpp>
#include <unet/detail/queue.hpp>
#include <unet/detail/raw_socket.hpp>
#include <unet/detail/socket_set.hpp>
#include <unet/dev/dev.hpp>
#include <unet/options.hpp>

namespace unet {

class Stack : public detail::NonMovable {
 public:
  Stack(std::unique_ptr<Dev> dev, Options opts = Options{});

  void runLoop();

  void stopLoop();

 private:
  void runLoopOnce();

  void sendLoop();

  void readLoop();

  std::unique_ptr<Dev> dev_;
  Options opts_;
  detail::SocketSet socketSet_;
  detail::Queue sendQueue_;
  detail::List<detail::RawSocket> ethernetSockets_;
  bool runningLoop_ = false;

  friend class RawSocket;
};

}  // namespace unet
