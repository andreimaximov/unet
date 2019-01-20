#include <unet/raw_socket.hpp>

namespace unet {

RawSocket::RawSocket(Stack& stack,
                     std::function<void(RawSocket&, std::uint32_t)> callback) {
  socket_ =
      new detail::RawSocket{stack.opts_.rawSocketSendQueueLen,
                            stack.opts_.rawSocketReadQueueLen,
                            stack.dev_->maxTransmissionUnit(),
                            stack.ethernetSockets_,
                            stack.socketSet_,
                            [this, callback](auto ev) { callback(*this, ev); }};
}

RawSocket::~RawSocket() {
  socket_->close();
}

std::size_t RawSocket::send(const std::uint8_t* buf, std::size_t bufLen) {
  return socket_->send(buf, bufLen);
}

std::size_t RawSocket::read(std::uint8_t* buf, std::size_t bufLen) {
  return socket_->read(buf, bufLen);
}

void RawSocket::subscribe(Event event) {
  socket_->subscribedEventMaskAdd(detail::eventAsInt(event));
}

void RawSocket::unsubscribe(Event event) {
  socket_->subscribedEventMaskRemove(detail::eventAsInt(event));
}

}  // namespace unet
