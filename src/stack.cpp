#include <unet/stack.hpp>

#include <boost/scope_exit.hpp>

#include <unet/exception.hpp>

namespace unet {

Stack::Stack(std::unique_ptr<Dev> dev, EthernetAddr ethAddr,
             Ipv4AddrCidr ipv4AddrCidr, Options opts)
    : dev_{std::move(dev)},
      ethAddr_{ethAddr},
      ipv4AddrCidr_{ipv4AddrCidr},
      opts_{opts},
      sendQueue_{opts.stackSendQueueLen} {
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

std::unique_ptr<Timer> Stack::createTimer(std::function<void()> f) {
  return std::make_unique<Timer>(timerManager_, f);
}

EthernetAddr Stack::getHwAddr() const {
  return ethAddr_;
}

Ipv4Addr Stack::getIpv4Addr() const {
  return *ipv4AddrCidr_;
}

void Stack::runLoopOnce() {
  readLoop();
  timerManager_.run();
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
