#include <unet/detail/socket.hpp>

#include <unet/detail/socket_set.hpp>

namespace unet {
namespace detail {

Socket::Socket(SocketSet& socketSet, std::size_t sendQueueLen,
               Queue::Policy sendQueuePolicy, Callback callback)
    : socketSet_{socketSet},
      sendQueue_{sendQueueLen, sendQueuePolicy},
      callback_{new Callback{callback}},
      ownerHook_{this},
      callbackHook_{this},
      dispatchHook_{this},
      dirtyHook_{this} {
  socketSet_.sockets_.push_back(ownerHook_);
}

bool Socket::hasCapacity(std::size_t capacity) const {
  return sendQueue_.hasCapacity(capacity);
}

bool Socket::hasQueuedFrames() {
  return !!sendQueue_.peek();
}

void Socket::sendFrame(std::unique_ptr<Frame>& f) {
  if (!f) {
    return;
  }

  auto hadFrame = !!sendQueue_.peek();
  sendQueue_.push(f);
  if (!hadFrame && !f) {
    socketSet_.dirty_.push_back(dirtyHook_);
  }
}

std::unique_ptr<Frame> Socket::popFrame() {
  auto f = sendQueue_.pop();
  if (!f) {
    return {};
  }

  // We unlink and then conditionally schedule the socket at the end of the
  // dirty list to ensure round-robin scheduling in SocketSet.
  dirtyHook_.unlink();
  if (sendQueue_.peek()) {
    socketSet_.dirty_.push_back(dirtyHook_);
  }

  // It is safe for the socket to be destroyed or the dirty state to change in
  // the callback since we do not rely on the socket continuing to exist in the
  // round-robin loop.
  onFramePopped();
  return f;
}

void Socket::subscribedEventMaskAdd(std::uint32_t mask) {
  eventMasksUpdate(subscribedEventMask_ | mask, pendingEventMask_);
}

void Socket::subscribedEventMaskRemove(std::uint32_t mask) {
  eventMasksUpdate(subscribedEventMask_ & (~mask), pendingEventMask_);
}

void Socket::pendingEventMaskAdd(std::uint32_t mask) {
  eventMasksUpdate(subscribedEventMask_, pendingEventMask_ | mask);
}

void Socket::pendingEventMaskRemove(std::uint32_t mask) {
  eventMasksUpdate(subscribedEventMask_, pendingEventMask_ & (~mask));
}

void Socket::destroy() {
  delete this;
}

void Socket::eventMasksUpdate(std::uint32_t subscribedEventMask,
                              std::uint32_t pendingEventMask) {
  auto callbackAttached = (subscribedEventMask_ & pendingEventMask_) != 0;
  auto callbackToggle =
      callbackAttached != ((subscribedEventMask & pendingEventMask) != 0);

  subscribedEventMask_ = subscribedEventMask;
  pendingEventMask_ = pendingEventMask;

  if (!callbackToggle) {
    return;
  } else if (callbackAttached) {
    callbackHook_.unlink();
  } else {
    socketSet_.callbacks_.push_back(callbackHook_);
  }
}

}  // namespace detail
}  // namespace unet
