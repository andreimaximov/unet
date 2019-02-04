#include <unet/detail/raw_socket.hpp>

#include <algorithm>

#include <unet/event.hpp>
#include <unet/exception.hpp>
#include <unet/wire/ethernet.hpp>
#include <unet/wire/ipv4.hpp>

namespace unet {
namespace detail {

RawSocket::RawSocket(std::uint32_t socketType, std::size_t sendQueueLen,
                     std::size_t readQueueLen, std::size_t maxTransmissionUnit,
                     EthernetAddr ethAddr, List<RawSocket>& sockets,
                     SocketSet& socketSet, Callback callback)
    : Socket{socketSet, sendQueueLen, callback},
      socketType_{socketType},
      socketsHook_{this},
      readQueue_{readQueueLen},
      maxTransmissionUnit_{maxTransmissionUnit},
      ethAddr_{ethAddr} {
  if (socketType_ != kEthernet && socketType_ != kIpv4) {
    throw Exception{"Unknown socket type."};
  }

  if ((socketType_ == kEthernet &&
       maxTransmissionUnit_ < sizeof(EthernetHeader)) ||
      (socketType_ == kIpv4 &&
       maxTransmissionUnit_ < sizeof(EthernetHeader) + sizeof(Ipv4Header))) {
    throw Exception{"MTU is too small for the specified layer."};
  }

  sockets.push_back(socketsHook_);
  if (hasCapacity()) {
    pendingEventMaskAdd(eventAsInt(Event::Send));
  }
}

void RawSocket::onFramePopped() {
  if (closed_ && !hasQueuedFrames()) {
    destroy();
    return;
  } else if (!closed_) {
    pendingEventMaskAdd(eventAsInt(Event::Send));
  }
}

void RawSocket::process(const Frame& f) {
  if (closed_ || !readQueue_.hasCapacity() ||
      (socketType_ == kIpv4 && !f.net)) {
    return;
  }

  auto copy = Frame::makeCopy(f);
  readQueue_.push(copy);
  pendingEventMaskAdd(eventAsInt(Event::Read));
}

std::size_t RawSocket::send(const std::uint8_t* buf, std::size_t bufLen) {
  if (!hasCapacity() ||
      (socketType_ == kEthernet && bufLen < sizeof(EthernetHeader)) ||
      (socketType_ == kIpv4 && bufLen < sizeof(Ipv4Header))) {
    return 0;
  }

  std::unique_ptr<Frame> f;
  std::size_t copyLen = 0;

  switch (socketType_) {
    case kEthernet:
      copyLen = std::min(maxTransmissionUnit_, bufLen);
      f = Frame::makeUninitialized(copyLen);
      std::copy(buf, buf + copyLen, f->data);
      break;
    case kIpv4:
      copyLen = std::min(maxTransmissionUnit_ - sizeof(EthernetHeader), bufLen);
      f = Frame::makeUninitialized(sizeof(EthernetHeader) + copyLen);
      f->dataAs<EthernetHeader>()->srcAddr = ethAddr_;
      f->dataAs<EthernetHeader>()->ethType = eth_type::kIpv4;
      f->net = f->data + sizeof(EthernetHeader);
      f->netLen = f->dataLen - sizeof(EthernetHeader);
      f->doIpv4Routing = true;
      std::copy(buf, buf + copyLen, f->net);
      break;
  }

  sendFrame(f);

  if (!hasCapacity()) {
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
    return;
  } else {
    // Wait for all egress frames (which we have confirmed to the user as
    // "sent") to be drained before destroying the socket. We shold no longer
    // invoke the callback for this socket.
    pendingEventMaskRemove(0xffff);
    closed_ = true;
  }
}

}  // namespace detail
}  // namespace unet
