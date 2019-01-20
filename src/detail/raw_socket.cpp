#include <unet/detail/raw_socket.hpp>

#include <algorithm>

#include <unet/event.hpp>

namespace unet {
namespace detail {

RawSocket::RawSocket(std::size_t sendQueueLen, std::size_t readQueueLen,
                     std::size_t maxTransmissionUnit, List<RawSocket>& sockets,
                     SocketSet& socketSet, Callback callback)
    : Socket{socketSet, sendQueueLen, callback},
      socketsHook_{this},
      readQueue_{readQueueLen},
      maxTransmissionUnit_{maxTransmissionUnit} {
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
  if (closed_ || !readQueue_.hasCapacity()) {
    return;
  }

  auto copy = Frame::make(f);
  readQueue_.push(copy);
  pendingEventMaskAdd(eventAsInt(Event::Read));
}

std::size_t RawSocket::send(const std::uint8_t* buf, std::size_t bufLen) {
  if (!hasCapacity() || bufLen == 0) {
    return 0;
  }

  auto copyLen = std::min(maxTransmissionUnit_, bufLen);
  auto f = Frame::make(copyLen);
  std::copy(buf, buf + copyLen, f->data);
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

  auto copyLen = std::min(bufLen, f->dataLen);
  std::copy(f->data, f->data + copyLen, buf);

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
