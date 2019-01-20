#include <unet/unet.hpp>

using namespace unet;

std::unique_ptr<Stack> stack;

// TODO(amaximov): Make this a real arping program once the timer and Ethernet +
// ARP representations are ready.
static const std::uint8_t kArpReq[] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x06, 0x11, 0x22, 0x33, 0x44,
    0x55, 0x08, 0x06, 0x00, 0x01, 0x08, 0x00, 0x06, 0x04, 0x00, 0x01,
    0x06, 0x11, 0x22, 0x33, 0x44, 0x55, 0x0A, 0xFF, 0xFF, 0x66, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0A, 0xFF, 0xFF, 0x65};

static void callback(RawSocket& socket, std::uint32_t mask) {
  if (mask & Event::Send &&
      socket.send(kArpReq, sizeof(kArpReq)) == sizeof(kArpReq)) {
    socket.unsubscribe(Event::Send);
    socket.subscribe(Event::Read);
  }

  if (mask & Event::Read) {
    stack->stopLoop();
  }
}

int main() {
  stack = std::make_unique<Stack>(std::make_unique<Tap>("tap0"));
  RawSocket socket{*stack, callback};
  socket.subscribe(Event::Send);
  stack->runLoop();
}
