#include <unet/stack.hpp>

#include <boost/scope_exit.hpp>

#include <unet/exception.hpp>
#include <unet/wire/arp.hpp>

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
    throw Exception{"runLoop(...) is already running."};
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
    f->net = nullptr;
    f->netLen = 0;
    process(*f);
  }
}

void Stack::process(detail::Frame& f) {
  if (f.dataLen < sizeof(EthernetHeader)) {
    return;
  }

  f.net = f.data + sizeof(EthernetHeader);
  f.netLen = f.dataLen - sizeof(EthernetHeader);

  // This loop is safe because RawSocket::process(...) is guaranteed to not
  // destroy the socket.
  for (detail::Hook<detail::RawSocket>& hook : ethernetSockets_) {
    hook->process(f);
  }

  if (f.dataAs<EthernetHeader>()->ethType == eth_type::kArp) {
    processArp(f);
  }
}

void Stack::processArp(detail::Frame& f) {
  if (f.netLen < sizeof(ArpHeader)) {
    return;
  }

  auto eth = f.dataAs<EthernetHeader>();
  auto arp = f.netAs<ArpHeader>();

  if (arp->hwType != arp_hw_addr::kEth ||
      arp->protoType != arp_proto_addr::kIpv4 || arp->hwLen != 6 ||
      arp->protoLen != 4 || arp->srcHwAddr != eth->srcAddr ||
      arp->dstProtoAddr != *ipv4AddrCidr_) {
    return;
  } else if (arp->op == arp_op::kRequest) {
    sendArp(arp->srcProtoAddr, arp->srcHwAddr, arp_op::kReply);
  } else if (arp->op == arp_op::kReply) {
    arpQueue_.add(arp->srcProtoAddr, arp->srcHwAddr);
  }
}

void Stack::sendArp(Ipv4Addr dstIpv4Addr, EthernetAddr dstHwAddr,
                    std::uint16_t arpOp) {
  if (!sendQueue_->hasCapacity()) {
    return;
  }

  auto f = detail::Frame::make(sizeof(EthernetHeader) + sizeof(ArpHeader));

  auto eth = f->dataAs<EthernetHeader>();
  eth->srcAddr = ethAddr_;
  eth->dstAddr = dstHwAddr;
  eth->ethType = eth_type::kArp;

  f->net = f->data + sizeof(EthernetHeader);
  f->netLen = f->dataLen - sizeof(EthernetHeader);

  auto arp = f->netAs<ArpHeader>();
  arp->hwType = arp_hw_addr::kEth;
  arp->protoType = arp_proto_addr::kIpv4;
  arp->hwLen = 6;
  arp->protoLen = 4;
  arp->op = arpOp;
  arp->srcHwAddr = ethAddr_;
  arp->srcProtoAddr = *ipv4AddrCidr_;
  arp->dstHwAddr = dstHwAddr;
  arp->dstProtoAddr = dstIpv4Addr;

  sendQueue_->push(f);
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
