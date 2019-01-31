#include <unet/example-main.hpp>

static void callback(RawSocket& socket, std::uint32_t mask) {
  std::uint8_t buf[sizeof(Ipv4Header)];

  auto ipv4 = reinterpret_cast<Ipv4Header*>(buf);
  ipv4->ihl = 5;
  ipv4->version = 4;
  ipv4->len = hostToNet<std::uint16_t>(sizeof(buf));
  ipv4->ttl = 64;
  ipv4->proto = 1;
  ipv4->srcAddr = stack->getIpv4Addr();
  ipv4->dstAddr = kDefaultGateway;
  ipv4->checksum = checksumIpv4(ipv4);

  if (socket.send(buf, sizeof(buf)) == sizeof(Ipv4Header)) {
    socket.unsubscribe(Event::Send);
  }
}

// TODO(amaximov): Create a ping example.
void runApp() {
  RawSocket socket{*stack, RawSocket::kIpv4, callback};
  socket.subscribe(Event::Send);

  stack->runLoop();
}
