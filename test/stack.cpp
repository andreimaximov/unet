#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <unet/stack.hpp>

namespace unet {

using testing::Invoke;
using testing::MockFunction;
using testing::NiceMock;

class MockDev : public Dev {
 public:
  MOCK_METHOD2(send, std::size_t(const std::uint8_t*, std::size_t));
  MOCK_METHOD2(read, std::size_t(std::uint8_t*, std::size_t));
  MOCK_CONST_METHOD0(maxTransmissionUnit, std::size_t());
};

TEST(StackTest, StopLoop) {
  Stack stack(std::make_unique<NiceMock<MockDev>>(), EthernetAddr{},
              Ipv4AddrCidr{Ipv4Addr{}, 32});

  MockFunction<void()> f;
  auto timer = stack.createTimer(f.AsStdFunction());
  timer->runAfter(std::chrono::seconds{0});

  EXPECT_CALL(f, Call()).WillOnce(Invoke([&stack]() { stack.stopLoop(); }));

  stack.runLoop();
}

}  // namespace unet
