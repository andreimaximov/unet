#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>

#include <unet/detail/frame.hpp>
#include <unet/detail/list.hpp>
#include <unet/detail/queue.hpp>

namespace unet {
namespace detail {

class SocketSet;

// A base class providing event subscriptions, frame scheduling, etc. for
// concrete sockets.
class Socket {
 public:
  using Callback = std::function<void(std::uint32_t)>;

  // Creates a socket w/a lifetime upper bounded by that of the socket set.
  // The callback will be dispatched whenever the subscribed and pending
  // event masks have a non-zero bitwise and. The socket MUST be allocated
  // on the heap w/operator new for destroy(...) to work. All methods of the
  // public API are safe to call inline from this or any other socket's
  // callback.
  Socket(SocketSet& socketSet, std::size_t sendQueueLen, Callback callback);

  Socket() = delete;
  Socket(const Socket&) = delete;
  Socket(Socket&&) = delete;
  Socket& operator=(const Socket&) = delete;
  Socket& operator=(Socket&&) = delete;

  // A callback invoked when a frame is popped. This function is allowed to
  // mutate this or any other socket in any way.
  virtual void onFramePopped() {}

  bool hasCapacity();

  bool hasQueuedFrames();

  void sendFrame(std::unique_ptr<Frame>& f);

  std::unique_ptr<Frame> popFrame();

  void subscribedEventMaskAdd(std::uint32_t mask);

  void subscribedEventMaskRemove(std::uint32_t mask);

  void pendingEventMaskAdd(std::uint32_t mask);

  void pendingEventMaskRemove(std::uint32_t mask);

  void destroy();

 protected:
  virtual ~Socket() = default;

 private:
  void eventMasksUpdate(std::uint32_t subscribedEventMask,
                        std::uint32_t pendingEventMask);

  SocketSet& socketSet_;
  Queue sendQueue_;
  std::shared_ptr<Callback> callback_;
  Hook<Socket> ownerHook_;
  Hook<Socket> callbackHook_;
  Hook<Socket> dispatchHook_;
  Hook<Socket> dirtyHook_;
  std::uint32_t subscribedEventMask_ = 0;
  std::uint32_t pendingEventMask_ = 0;
  std::uint32_t dispatchEventMask_ = 0;

  friend class SocketSet;
};

}  // namespace detail
}  // namespace unet
