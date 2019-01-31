#include <algorithm>
#include <vector>

#include <unet/example-main.hpp>

DEFINE_string(addr, "", "The IPv4 address to ping.");
DEFINE_validator(addr, &validateIpv4Addr);
DEFINE_uint32(count, 64, "The number of pings to send.");
DEFINE_uint32(payload, 56, "The number of data bytes to send.");

bool gotReply = false;
std::uint16_t pingId = randInt<std::uint16_t>();
std::uint16_t pingSeq = 1;
Ipv4Addr dstAddr{};
std::chrono::steady_clock::time_point sentAt{};
std::unique_ptr<Timer> sendAgain;
std::vector<std::uint8_t> payload;

static void makeRandPayload() {
  for (std::uint8_t& b : payload) {
    b = randInt<std::uint8_t>();
  }
}

static bool checkIpv4(const std::uint8_t* buf, std::size_t bufLen,
                      std::uint8_t& ttl) {
  auto ipv4 = reinterpret_cast<const Ipv4Header*>(buf);
  std::size_t headerLen = ipv4->ihl * 4;

  if (ipv4->proto != ipv4_proto::kIcmp ||
      bufLen < headerLen + sizeof(Icmpv4Header) + FLAGS_payload) {
    return false;
  }

  auto icmp = reinterpret_cast<const Icmpv4Header*>(buf + headerLen);
  if (icmp->type != 0 || icmp->code != 0) {
    return false;
  }

  auto echo = reinterpret_cast<const Icmpv4Echo*>(icmp->data);
  if (echo->id != hostToNet(pingId) || echo->seq != hostToNet(pingSeq)) {
    return false;
  }

  if (!std::equal(buf + headerLen + sizeof(Icmpv4Header),
                  buf + headerLen + sizeof(Icmpv4Header) + FLAGS_payload,
                  payload.data())) {
    return false;
  }

  ttl = ipv4->ttl;
  return true;
}

static void sendIpv4(RawSocket& socket) {
  std::vector<std::uint8_t> buf(sizeof(Ipv4Header) + sizeof(Icmpv4Header) +
                                FLAGS_payload);

  auto ipv4 = reinterpret_cast<Ipv4Header*>(buf.data());
  ipv4->ihl = 5;
  ipv4->version = 4;
  ipv4->len = hostToNet<std::uint16_t>(buf.size());
  ipv4->ttl = 64;
  ipv4->proto = ipv4_proto::kIcmp;
  ipv4->srcAddr = stack->getIpv4Addr();
  ipv4->dstAddr = dstAddr;

  auto icmp = reinterpret_cast<Icmpv4Header*>(buf.data() + sizeof(Ipv4Header));
  icmp->type = 8;
  icmp->code = 0;

  auto echo = reinterpret_cast<Icmpv4Echo*>(icmp->data);
  echo->id = hostToNet(pingId);
  echo->seq = hostToNet(pingSeq);

  makeRandPayload();
  std::copy(payload.begin(), payload.end(),
            buf.data() + sizeof(Ipv4Header) + sizeof(Icmpv4Header));

  icmp->checksum = checksumIcmpv4(icmp, FLAGS_payload);
  ipv4->checksum = checksumIpv4(ipv4);

  if (socket.send(buf.data(), buf.size()) == 0) {
    return;
  }

  sentAt = std::chrono::steady_clock::now();
  sendAgain->runAfter(std::chrono::seconds{1});
  socket.unsubscribe(Event::Send);
}

static void readIpv4(RawSocket& socket) {
  static std::uint8_t buf[1500];

  std::size_t readLen;
  std::uint8_t ttl;
  while ((readLen = socket.read(buf, sizeof(buf))) > 0) {
    if (!checkIpv4(buf, readLen, ttl)) {
      continue;
    }

    socket.unsubscribe(Event::Read);
    gotReply = true;

    auto waited = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - sentAt);
    std::cout
        << boost::format{"%1% bytes from %2%: icmp_seq=%3% ttl=%4% time=%5% ms"}
            % (sizeof(Icmpv4Header) + FLAGS_payload)
            % dstAddr
            % pingSeq
            % static_cast<std::uint32_t>(ttl)
            % waited.count()
        << std::endl;

    return;
  }
}

static void callback(RawSocket& socket, std::uint32_t mask) {
  if (mask & Event::Send) {
    sendIpv4(socket);
  }

  if (mask & Event::Read) {
    readIpv4(socket);
  }
}

void runApp() {
  dstAddr = parseIpv4(FLAGS_addr);
  payload = std::vector<std::uint8_t>(FLAGS_payload);

  RawSocket socket{*stack, RawSocket::kIpv4, callback};
  socket.subscribe(Event::Send);
  socket.subscribe(Event::Read);

  sendAgain = stack->createTimer([&socket]() {
    if (!gotReply) {
      std::cout << "Timeout" << std::endl;
    }

    if (pingSeq++ == FLAGS_count) {
      stack->stopLoop();
    }

    socket.subscribe(Event::Send);
    socket.subscribe(Event::Read);
    gotReply = false;
  });

  stack->runLoop();
}
