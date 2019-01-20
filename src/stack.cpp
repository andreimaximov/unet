#include <unet/stack.hpp>

#include <boost/scope_exit.hpp>

#include <unet/exception.hpp>

namespace unet {

Stack::Stack(std::unique_ptr<Dev> dev, Options opts)
    : dev_{std::move(dev)}, opts_{opts}, sendQueue_{opts.stackSendQueueLen} {
  if (!dev_) {
    throw Exception{"Invalid dev."};
  }
}

void Stack::runLoop() {
  if (runningLoop_) {
    throw Exception{"runLoopOnce(...) is already running."};
  };

  runningLoop_ = true;

  BOOST_SCOPE_EXIT(&runningLoop_) { runningLoop_ = false; }
  BOOST_SCOPE_EXIT_END

  while (runningLoop_) {
    runLoopOnce();
  }
}

void Stack::stopLoop() {
  runningLoop_ = false;
}

void Stack::runLoopOnce() {
  readLoop();
  socketSet_.dispatch();
  socketSet_.drainRoundRobin(sendQueue_);
  sendLoop();
}

void Stack::sendLoop() {
  boost::optional<detail::Frame&> f = boost::none;
  while ((f = sendQueue_.peek()) && (dev_->send(f->data, f->dataLen) != 0)) {
    sendQueue_.pop();
  }
}

void Stack::readLoop() {
  auto frameLen = dev_->maxTransmissionUnit();
  auto f = detail::Frame::make(frameLen);
  while ((f->dataLen = dev_->read(f->data, frameLen)) > 0) {
    // This loop is safe because RawSocket::process(...) is guaranteed to not
    // destroy the socket.
    for (detail::Hook<detail::RawSocket>& hook : ethernetSockets_) {
      hook->process(*f);
    }
  }
}

}  // namespace unet
