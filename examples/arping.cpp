#include <unet/example-main.hpp>

DEFINE_string(addr, "",
              "The IPv4 address to send Ethernet address requests for.");
DEFINE_validator(addr, &validateIpv4Addr);
DEFINE_uint32(count, 64, "The number of ARP requests to send.");

bool gotReply = false;
std::uint32_t arpSeq = 0;
Ipv4Addr dstProtoAddr{};
std::chrono::steady_clock::time_point sentAt{};
std::unique_ptr<Timer> sendAgain;

static void sendArp(RawSocket& socket) {
  static std::uint8_t buf[1500];

  auto eth = reinterpret_cast<EthernetHeader*>(buf);
  eth->dstAddr = kEthernetBcastAddr;
  eth->srcAddr = stack->getHwAddr();
  eth->ethType = eth_type::kArp;

  auto arp = reinterpret_cast<ArpHeader*>(buf + sizeof(EthernetHeader));
  arp->hwType = arp_hw_addr::kEth;
  arp->protoType = arp_proto_addr::kIpv4;
  arp->hwLen = 6;
  arp->protoLen = 4;
  arp->op = arp_op::kRequest;
  arp->srcHwAddr = stack->getHwAddr();
  arp->srcProtoAddr = stack->getIpv4Addr();
  arp->dstHwAddr = kEthernetBcastAddr;
  arp->dstProtoAddr = dstProtoAddr;

  if (socket.send(buf, sizeof(EthernetHeader) + sizeof(ArpHeader)) == 0) {
    return;
  }

  sentAt = std::chrono::steady_clock::now();
  sendAgain->runAfter(std::chrono::seconds{1});
  socket.unsubscribe(Event::Send);
}

static void readArp(RawSocket& socket) {
  static std::uint8_t buf[1500];

  while (socket.read(buf, sizeof(buf)) ==
         sizeof(EthernetHeader) + sizeof(ArpHeader)) {
    auto eth = reinterpret_cast<const EthernetHeader*>(buf);
    if (eth->dstAddr != stack->getHwAddr() || eth->ethType != eth_type::kArp) {
      return;
    }

    auto arp = reinterpret_cast<const ArpHeader*>(buf + sizeof(EthernetHeader));
    if (arp->hwType != arp_hw_addr::kEth ||
        arp->protoType != arp_proto_addr::kIpv4 || arp->hwLen != 6 ||
        arp->protoLen != 4 || arp->op != arp_op::kReply ||
        arp->srcHwAddr != eth->srcAddr || arp->srcProtoAddr != dstProtoAddr ||
        arp->dstHwAddr != stack->getHwAddr() ||
        arp->dstProtoAddr != stack->getIpv4Addr()) {
      return;
    }

    socket.unsubscribe(Event::Read);
    gotReply = true;

    auto waited = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - sentAt);
    std::cout
        << boost::format{"28 bytes from %1% (%2%) index=%3% time=%4% ms"} %
               arp->srcHwAddr % arp->srcProtoAddr % arpSeq % waited.count()
        << std::endl;

    return;
  }
}

static void callback(RawSocket& socket, std::uint32_t mask) {
  if (mask & Event::Send) {
    sendArp(socket);
  }

  if (mask & Event::Read) {
    readArp(socket);
  }
}

void runApp() {
  dstProtoAddr = parseIpv4(FLAGS_addr);

  RawSocket socket{*stack, RawSocket::kEthernet, callback};
  socket.subscribe(Event::Send);
  socket.subscribe(Event::Read);

  sendAgain = stack->createTimer([&socket]() {
    if (arpSeq++ == FLAGS_count) {
      stack->stopLoop();
      return;
    } else if (!gotReply) {
      std::cout << "Timeout" << std::endl;
    }

    socket.subscribe(Event::Send);
    socket.subscribe(Event::Read);
    gotReply = false;
  });

  stack->runLoop();
}
