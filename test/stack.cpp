#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <unet/exception.hpp>
#include <unet/stack.hpp>

namespace unet {

using testing::Invoke;
using testing::MockFunction;
using testing::NiceMock;
using testing::Test;

class MockDev : public Dev {
 public:
  MOCK_METHOD2(send, std::size_t(const std::uint8_t*, std::size_t));
  MOCK_METHOD2(read, std::size_t(std::uint8_t*, std::size_t));
  MOCK_CONST_METHOD0(maxTransmissionUnit, std::size_t());
};

class StackTest : public Test {
 public:
  StackTest()
      : stack{std::make_unique<NiceMock<MockDev>>(), EthernetAddr{},
              Ipv4AddrCidr{Ipv4Addr{}, 32}, Ipv4Addr{}} {}

  Stack stack;
};

TEST_F(StackTest, LoopRunStopAndRunAgain) {
  MockFunction<void()> f;
  auto timer = stack.createTimer(f.AsStdFunction());
  timer->runAfter(std::chrono::seconds{0});

  EXPECT_CALL(f, Call()).Times(2).WillRepeatedly(
      Invoke([this]() { stack.stopLoop(); }));

  stack.runLoop();

  timer->runAfter(std::chrono::seconds{0});
  stack.runLoop();
}

TEST_F(StackTest, LoopRunStopThrowAndRunAgain) {
  MockFunction<void()> f1;
  auto timer1 = stack.createTimer(f1.AsStdFunction());
  timer1->runAfter(std::chrono::seconds{0});

  EXPECT_CALL(f1, Call()).WillOnce(Invoke([this]() {
    stack.stopLoop();
    stack.runLoop();
  }));

  ASSERT_THROW(stack.runLoop(), Exception);

  MockFunction<void()> f2;
  auto timer2 = stack.createTimer(f2.AsStdFunction());
  timer2->runAfter(std::chrono::seconds{0});

  EXPECT_CALL(f2, Call()).WillOnce(Invoke([this]() { stack.stopLoop(); }));

  stack.runLoop();
}

}  // namespace unet
