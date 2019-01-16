#include <unet/detail/socket_set.hpp>

#include <boost/scope_exit.hpp>

#include <unet/detail/socket.hpp>

namespace unet {
namespace detail {

SocketSet::~SocketSet() {
  BOOST_ASSERT(!dispatching_);
  while (!sockets_.empty()) {
    Hook<Socket>& hook = sockets_.front();
    sockets_.pop_front();
    hook->destroy();
  }
}

void SocketSet::dispatch() {
  BOOST_ASSERT(!dispatching_);

  dispatching_ = true;
  List<Socket> dispatcher;

  BOOST_SCOPE_EXIT(&dispatching_, &dispatcher) {
    dispatching_ = false;
    dispatcher.clear();
  }
  BOOST_SCOPE_EXIT_END

  for (Hook<Socket>& hook : callbacks_) {
    // Save the event mask to dispatch since a callback on one socket can
    // perform a mutation which changes the pending event mask of another socket
    // which is dispatched later.
    dispatcher.push_back(hook->dispatchHook_);
    hook->dispatchEventMask_ =
        hook->subscribedEventMask_ & hook->pendingEventMask_;
  }

  while (!dispatcher.empty()) {
    Hook<Socket>& hook = dispatcher.front();
    dispatcher.pop_front();

    // We acquire a reference on the callback to ensure it stays alive even if
    // the socket is destroyed inline.
    auto callback = hook->callback_;
    (*callback)(hook->dispatchEventMask_);
  }
}

void SocketSet::drainRoundRobin(Queue& queue) {
  while (queue.hasCapacity() && !dirty_.empty()) {
    Hook<Socket>& hook = dirty_.front();
    auto f = hook->popFrame();
    queue.push(f);
  }
}

}  // namespace detail
}  // namespace unet
