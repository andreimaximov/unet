#pragma once

#include <unet/event.hpp>
#include <unet/exception.hpp>

namespace unet {

template <typename T>
class SocketBase {
 public:
  SocketBase(T* socket) : socket_{socket} {}

  SocketBase() = default;
  SocketBase(const SocketBase&) = delete;
  SocketBase(SocketBase&& other) { *this = std::move(other); }
  SocketBase& operator=(const SocketBase&) = delete;
  SocketBase& operator=(SocketBase&& other) {
    if (socket_) {
      socket_->close();
    }
    socket_ = other.socket_;
    other.socket_ = nullptr;
  }

  ~SocketBase() {
    if (socket_) {
      socket_->close();
    }
  }

  // Schedules the socket callback to run once the event becomes pending.
  void subscribe(Event event) {
    socketSafe()->subscribedEventMaskAdd(detail::eventAsInt(event));
  }

  // Deschedules the socket callback from running when this is the only pending
  // event.
  void unsubscribe(Event event) {
    socketSafe()->subscribedEventMaskRemove(detail::eventAsInt(event));
  }

 protected:
  T* socketSafe() {
    if (!socket_) {
      throw Exception{"Socket unavailable!"};
    }
    return socket_;
  }

 private:
  T* socket_ = nullptr;
};

}  // namespace unet
