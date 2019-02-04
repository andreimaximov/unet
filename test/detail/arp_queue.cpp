#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <unet/detail/arp_queue.hpp>

namespace unet {
namespace detail {

using testing::_;
using testing::InSequence;
using testing::MockFunction;
using testing::Test;

constexpr Ipv4Addr kIpv4[] = {Ipv4Addr{10, 255, 255, 1},
                              Ipv4Addr{10, 255, 255, 2}};
constexpr EthernetAddr kEth[] = {
    EthernetAddr{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00},
    EthernetAddr{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x11}};
constexpr std::chrono::steady_clock::time_point kTpNowBase{};

class ArpQueueTest : public Test {
 public:
  void SetUp() override {
    sendQueue = std::make_shared<Queue>(2);
    timerManager = std::make_shared<TimerManager>(kTpNowBase);
    arpQueue = std::make_shared<ArpQueue>(2, 2, std::chrono::seconds{1},
                                          std::chrono::seconds{60}, sendQueue,
                                          timerManager);
  }

  Frame* Delay(Ipv4Addr hopAddr, bool shouldSendArp) {
    auto f = Frame::makeUninitialized(sizeof(EthernetHeader));
    f->hopAddr = hopAddr;
    auto p = f.get();
    EXPECT_EQ(arpQueue->delay(std::move(f)), shouldSendArp);

    // This can be a dangling pointer if the frame is tail dropped.
    return p;
  }

  std::shared_ptr<Queue> sendQueue;
  std::shared_ptr<TimerManager> timerManager;
  std::shared_ptr<ArpQueue> arpQueue;
};

TEST_F(ArpQueueTest, DelayAndAdd) {
  auto f1 = Delay(kIpv4[0], true);
  auto f2 = Delay(kIpv4[0], false);
  ASSERT_FALSE(sendQueue->pop());

  arpQueue->add(kIpv4[0], kEth[0]);

  auto f3 = sendQueue->pop();
  ASSERT_TRUE(f3);
  ASSERT_EQ(f3.get(), f1);
  ASSERT_EQ(f3->dataAs<EthernetHeader>()->dstAddr, kEth[0]);

  auto f4 = sendQueue->pop();
  ASSERT_TRUE(f4);
  ASSERT_EQ(f4.get(), f2);
  ASSERT_EQ(f4->dataAs<EthernetHeader>()->dstAddr, kEth[0]);

  ASSERT_FALSE(sendQueue->pop());
}

TEST_F(ArpQueueTest, DelayAndTimeout) {
  Delay(kIpv4[0], true);
  Delay(kIpv4[0], false);
  ASSERT_FALSE(sendQueue->pop());

  timerManager->run(kTpNowBase + std::chrono::seconds{2});

  auto f1 = Delay(kIpv4[1], true);
  arpQueue->add(kIpv4[1], kEth[1]);

  auto f2 = sendQueue->pop();
  ASSERT_TRUE(f2);
  ASSERT_EQ(f2.get(), f1);
  ASSERT_EQ(f2->dataAs<EthernetHeader>()->dstAddr, kEth[1]);
}

TEST_F(ArpQueueTest, DelayAndDrop) {
  Delay(kIpv4[0], true);
  Delay(kIpv4[0], false);
  sendQueue->pop();

  Delay(kIpv4[1], false);
  ASSERT_FALSE(sendQueue->pop());
}

TEST_F(ArpQueueTest, DropFrameBadDataLen) {
  auto f = Frame::makeUninitialized(1);
  ASSERT_FALSE(arpQueue->delay(std::move(f)));
  ASSERT_FALSE(sendQueue->pop());
}

}  // namespace detail
}  // namespace unet
