#include <unet/stack.hpp>

#include <boost/scope_exit.hpp>

#include <unet/exception.hpp>

namespace unet {

Stack::Stack(std::unique_ptr<Dev> dev, EthernetAddr ethAddr,
             Ipv4AddrCidr ipv4AddrCidr, Ipv4Addr defaultGateway, Options opts)
    : dev_{std::move(dev)},
      ethAddr_{ethAddr},
      ipv4AddrCidr_{ipv4AddrCidr},
      defaultGateway_{defaultGateway},
      opts_{opts},
      sendQueue_{std::make_shared<detail::Queue>(opts.stackSendQueueLen)},
      timerManager_{std::make_shared<TimerManager>()},
      arpQueue_{opts.arpQueueLen, opts.arpCacheSize, opts.arpTimeout,
                opts.arpCacheTTL, sendQueue_,        timerManager_} {
  if (!dev_) {
    throw Exception{"Invalid dev."};
  } else if (!ipv4AddrCidr.isInSubnet(defaultGateway)) {
    throw Exception{"Default gateway should be on the same subnet."};
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
  return std::make_unique<Timer>(*timerManager_, f);
}

EthernetAddr Stack::getHwAddr() const {
  return ethAddr_;
}

Ipv4Addr Stack::getIpv4Addr() const {
  return *ipv4AddrCidr_;
}

void Stack::runLoopOnce() {
  readLoop();
  timerManager_->run();
  socketSet_.dispatch();
  socketSet_.drainRoundRobin(*sendQueue_);
  sendLoop();
}

void Stack::sendLoop() {
  while (auto f = sendQueue_->peek()) {
    if (f->doIpv4Routing && !sendIpv4(*f)) {
      // Lookup of the Ethernet address for the next hop has failed. Send an
      // ARP request for the hop and delay the frame in the meantime.
      arpQueue_.delay(sendQueue_->pop());

      // TODO(amaximov): Send ARP request.
      continue;
    }

    if (dev_->send(f->data, f->dataLen) == 0) {
      return;
    }

    sendQueue_->pop();
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

bool Stack::sendIpv4(detail::Frame& f) {
  auto dstAddr = f.netAs<Ipv4Header>()->dstAddr;
  f.hopAddr = ipv4AddrCidr_.isInSubnet(dstAddr) ? dstAddr : defaultGateway_;

  auto ethAddr = arpQueue_.lookup(f.hopAddr);
  if (!ethAddr) {
    return false;
  }

  f.dataAs<EthernetHeader>()->dstAddr = *ethAddr;
  return true;
}

}  // namespace unet
