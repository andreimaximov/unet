#include <unet/raw_socket.hpp>

namespace unet {

RawSocket::RawSocket(Stack& stack, Type type,
                     std::function<void(RawSocket&, std::uint32_t)> callback)
    : SocketBase{new detail::RawSocket{
          (type == kEthernet) ? detail::RawSocket::kEthernet
                              : detail::RawSocket::kIpv4,
          stack.opts_.rawSocketSendQueueLen, stack.opts_.rawSocketReadQueueLen,
          stack.dev_->maxTransmissionUnit(), stack.serializer_,
          (type == kEthernet) ? stack.ethernetSockets_ : stack.ipv4Sockets_,
          stack.socketSet_,
          [this, callback](auto mask) { callback(*this, mask); }}} {}

std::size_t RawSocket::send(const std::uint8_t* buf, std::size_t bufLen) {
  return socketSafe()->send(buf, bufLen);
}

std::size_t RawSocket::read(std::uint8_t* buf, std::size_t bufLen) {
  return socketSafe()->read(buf, bufLen);
}

}  // namespace unet
