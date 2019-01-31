#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>

#include <unet/detail/nonmovable.hpp>
#include <unet/detail/raw_socket.hpp>
#include <unet/event.hpp>
#include <unet/stack.hpp>

namespace unet {

// A socket for communicating via Ethernet or IPv4 frames. The following events
// are supported:
//
// - Send
// - Read
class RawSocket : public detail::NonMovable {
 public:
  // The type of raw socket which determines the layer of sent and received
  // frames.
  enum class Type {
    Ethernet,
    Ipv4,
  };

  static constexpr auto kEthernet = Type::Ethernet;
  static constexpr auto kIpv4 = Type::Ipv4;

  RawSocket() = delete;

  // Creates a raw socket bound to the provided stack. The socket MUST NOT
  // exceed the lifetime of the stack. The callback is invoked once a
  // subscribed event becomes pending.
  RawSocket(Stack& stack, Type type,
            std::function<void(RawSocket&, std::uint32_t)> callback);

  ~RawSocket();

  // Sends a frame represented by buf.
  //
  // Return the number of bytes sent. Sending 0 bytes for a non-zero length buf
  // indicates the socket is exhausted. Sending > 0 but < bufLen bytes indicates
  // the frame was truncated to respect the maximum transmission unit of the
  // underlying device. The buf should be at least as long as the header of the
  // specified layer.
  std::size_t send(const std::uint8_t* buf, std::size_t bufLen);

  // Reads a frame into buf. The frame will be truncated if buf is not long
  // enough.
  //
  // Return the number of bytes read into buf.
  std::size_t read(std::uint8_t* buf, std::size_t bufLen);

  // Schedules the socket callback to run once an event becomes pending.
  void subscribe(Event event);

  // Deschedules the socket callback from running when an event becomes pending.
  void unsubscribe(Event event);

 private:
  detail::RawSocket* socket_ = nullptr;
};

}  // namespace unet
