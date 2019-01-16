#pragma once

#include <unet/detail/list.hpp>
#include <unet/detail/nonmovable.hpp>
#include <unet/detail/queue.hpp>

namespace unet {
namespace detail {

class Socket;

// A SocketSet owns and manages a set of sockets.
class SocketSet : public NonMovable {
 public:
  SocketSet() = default;

  ~SocketSet();

  // Invokes callbacks for all sockets managed by the socket set which have a
  // non-zero bitwise and of the subscribed and pending event masks.
  void dispatch();

  // Pops as many frames as possible from sockets managed by this socket set to
  // the queue in round robin fashion.
  void drainRoundRobin(Queue& queue);

 private:
  List<Socket> sockets_;
  List<Socket> callbacks_;
  List<Socket> dirty_;
  bool dispatching_ = false;

  friend class Socket;
};

}  // namespace detail
}  // namespace unet
