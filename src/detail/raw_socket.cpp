#include <unet/detail/raw_socket.hpp>

#include <algorithm>

#include <unet/event.hpp>
#include <unet/exception.hpp>
#include <unet/wire/ethernet.hpp>
#include <unet/wire/ipv4.hpp>

namespace unet {
namespace detail {

static auto socketTypePolicy(std::uint32_t socketType) {
  return (socketType == RawSocket::kEthernet) ? Queue::Policy::DataLen
                                              : Queue::Policy::NetLen;
}

RawSocket::RawSocket(std::uint32_t socketType, std::size_t sendQueueLen,
                     std::size_t readQueueLen, std::size_t maxTransmissionUnit,
                     std::shared_ptr<Serializer> serializer,
                     List<RawSocket>& sockets, SocketSet& socketSet,
                     Callback callback)
    : Socket{socketSet, sendQueueLen, socketTypePolicy(socketType), callback},
      socketType_{socketType},
      socketsHook_{this},
      readQueue_{readQueueLen, socketTypePolicy(socketType)},
      sendLenMax_{0},
      serializer_{serializer} {
  if (socketType_ != kEthernet && socketType_ != kIpv4) {
    throw Exception{"Unknown socket type."};
  }

  auto headersLen = (socketType_ == kEthernet)
                        ? sizeof(EthernetHeader)
                        : sizeof(EthernetHeader) + sizeof(Ipv4Header);

  if (maxTransmissionUnit < headersLen) {
    throw Exception{"MTU is too small for the specified layer."};
  } else if (socketType_ == kEthernet) {
    sendLenMax_ = maxTransmissionUnit;
  } else if (socketType_ == kIpv4) {
    sendLenMax_ = maxTransmissionUnit - sizeof(EthernetHeader);
  }

  sockets.push_back(socketsHook_);
  if (hasCapacity(1)) {
    pendingEventMaskAdd(eventAsInt(Event::Send));
  }
}

void RawSocket::onFramePopped() {
  if (closed_ && !hasQueuedFrames()) {
    destroy();
  } else if (!closed_) {
    // Sent frames have non-zero length so we don't need to do a capacity check.
    pendingEventMaskAdd(eventAsInt(Event::Send));
  }
}

void RawSocket::process(const Frame& f) {
  if (closed_ || !readQueue_.hasCapacity(f) ||
      (socketType_ == kIpv4 && !f.net)) {
    return;
  }

  auto copy = Frame::makeCopy(f);
  readQueue_.push(copy);
  pendingEventMaskAdd(eventAsInt(Event::Read));
}

std::size_t RawSocket::send(const std::uint8_t* buf, std::size_t bufLen) {
  auto headerLen =
      (socketType_ == kEthernet) ? sizeof(EthernetHeader) : sizeof(Ipv4Header);

  if (bufLen < headerLen) {
    return 0;
  }

  auto copyLen = std::min(sendLenMax_, bufLen);
  if (copyLen == 0 || !hasCapacity(copyLen)) {
    return 0;
  }

  std::unique_ptr<Frame> f;

  switch (socketType_) {
    case kEthernet:
      f = Frame::makeBuf(buf, copyLen);
      break;
    case kIpv4:
      f = serializer_->make(copyLen, [buf, copyLen](Frame& f) {
        f.dataAs<EthernetHeader>()->ethType = eth_type::kIpv4;
        std::copy(buf, buf + copyLen, f.net);
        f.doIpv4Routing = true;
      });
      break;
  }

  sendFrame(f);

  if (!hasCapacity(1)) {
    pendingEventMaskRemove(eventAsInt(Event::Send));
  }

  return copyLen;
}

std::size_t RawSocket::read(std::uint8_t* buf, std::size_t bufLen) {
  auto f = readQueue_.pop();
  if (!f) {
    return 0;
  }

  auto readBuf = (socketType_ == kEthernet) ? f->data : f->net;
  auto readLen = (socketType_ == kEthernet) ? f->dataLen : f->netLen;
  auto copyLen = std::min(bufLen, readLen);
  std::copy(readBuf, readBuf + copyLen, buf);

  if (!readQueue_.peek()) {
    pendingEventMaskRemove(eventAsInt(Event::Read));
  }

  return copyLen;
}

void RawSocket::close() {
  if (!hasQueuedFrames()) {
    destroy();
  } else {
    // Wait for all egress frames (which we have confirmed to the user as
    // "sent") to be drained before destroying the socket. We should no longer
    // invoke the callback for this socket.
    pendingEventMaskRemove(0xffff);
    socketsHook_.unlink();
    closed_ = true;
  }
}

}  // namespace detail
}  // namespace unet
