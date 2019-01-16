#include <exception>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <unet/detail/socket.hpp>
#include <unet/detail/socket_set.hpp>

namespace unet {
namespace detail {

using testing::InSequence;
using testing::Invoke;
using testing::MockFunction;
using testing::NiceMock;
using testing::StrictMock;
using testing::Test;
using testing::Throw;

using MockCallback = StrictMock<MockFunction<void(std::uint32_t)>>;

class MockSocket : public Socket {
 public:
  MockSocket(SocketSet& socketSet, Callback callback)
      : Socket{socketSet, 1024, callback} {}

  MOCK_METHOD0(onFramePopped, void());
};

class SocketTest : public Test {
 public:
  void SetUp() {
    s1 = new NiceMock<MockSocket>{ss, cb1.AsStdFunction()};
    s2 = new NiceMock<MockSocket>{ss, cb2.AsStdFunction()};
    s3 = new NiceMock<MockSocket>{ss, cb3.AsStdFunction()};
  }

  void ScheduleAll() {
    s1->pendingEventMaskAdd(1);
    s1->subscribedEventMaskAdd(1);
    s2->pendingEventMaskAdd(1);
    s2->subscribedEventMaskAdd(1);
    s3->pendingEventMaskAdd(1);
    s3->subscribedEventMaskAdd(1);
  }

  static void NoOp(std::uint32_t mask) {}

  void ExpectCallback1(std::uint32_t mask, Socket::Callback cb = NoOp) {
    EXPECT_CALL(cb1, Call(mask)).WillOnce(Invoke(cb));
  }

  void ExpectCallback2(std::uint32_t mask, Socket::Callback cb = NoOp) {
    EXPECT_CALL(cb2, Call(mask)).WillOnce(Invoke(cb));
  }

  void ExpectCallback3(std::uint32_t mask, Socket::Callback cb = NoOp) {
    EXPECT_CALL(cb3, Call(mask)).WillOnce(Invoke(cb));
  }

  void PushFrame(Socket& s, const char* data) {
    auto f = Frame::makeStr(data);
    s.sendFrame(f);
  }

  SocketSet ss;
  MockCallback cb1;
  MockCallback cb2;
  MockCallback cb3;
  NiceMock<MockSocket>* s1 = nullptr;
  NiceMock<MockSocket>* s2 = nullptr;
  NiceMock<MockSocket>* s3 = nullptr;
};

TEST_F(SocketTest, CallbackNoSubscriptionOrPending) {
  ss.dispatch();
}

TEST_F(SocketTest, CallbackNoSubscriptionButHasPending) {
  s1->pendingEventMaskAdd(1);
  ss.dispatch();
}

TEST_F(SocketTest, CallbackHasSubscriptionButNoPending) {
  s1->subscribedEventMaskAdd(1);
  ss.dispatch();
}

TEST_F(SocketTest, CallbackHasSubscriptionAndPending) {
  ExpectCallback1(1);

  s1->pendingEventMaskAdd(1);
  s1->subscribedEventMaskAdd(1);
  ss.dispatch();
}

TEST_F(SocketTest, CallbackMaskOnlySubscribed) {
  ExpectCallback1(1);

  s1->pendingEventMaskAdd(3);
  s1->subscribedEventMaskAdd(1);
  ss.dispatch();
}

TEST_F(SocketTest, CallbackException) {
  ScheduleAll();

  {
    InSequence s;
    ExpectCallback1(1, [](auto) { throw std::exception{}; });
    ExpectCallback1(1);
    ExpectCallback2(1);
    ExpectCallback3(1);
  }

  ASSERT_THROW(ss.dispatch(), std::exception);
  ss.dispatch();
}

TEST_F(SocketTest, UpdateSubscribed) {
  ScheduleAll();

  {
    InSequence s;
    ExpectCallback1(1);
    ExpectCallback2(1);
    ExpectCallback3(1);
    ExpectCallback1(1);
  }

  ss.dispatch();
  s1->subscribedEventMaskAdd(2);
  s2->subscribedEventMaskRemove(1);
  s3->subscribedEventMaskRemove(3);
  ss.dispatch();
}

TEST_F(SocketTest, UpdateSubscribedOfThisInline) {
  ScheduleAll();

  {
    InSequence s;
    ExpectCallback1(1, [this](auto) { s1->subscribedEventMaskAdd(2); });
    ExpectCallback2(1, [this](auto) { s2->subscribedEventMaskRemove(1); });
    ExpectCallback3(1, [this](auto) { s3->subscribedEventMaskRemove(3); });
    ExpectCallback1(1);
  }

  ss.dispatch();
  ss.dispatch();
}

TEST_F(SocketTest, UpdatePending) {
  ScheduleAll();
  s1->subscribedEventMaskAdd(3);
  s2->subscribedEventMaskAdd(3);
  s3->subscribedEventMaskAdd(3);

  {
    InSequence s;
    ExpectCallback1(1);
    ExpectCallback2(1);
    ExpectCallback3(1);
    ExpectCallback1(3);
  }

  ss.dispatch();
  s1->pendingEventMaskAdd(2);
  s2->pendingEventMaskRemove(1);
  s3->pendingEventMaskRemove(3);
  ss.dispatch();
}

TEST_F(SocketTest, UpdatePendingOfThisInline) {
  ScheduleAll();
  s1->subscribedEventMaskAdd(3);
  s2->subscribedEventMaskAdd(3);
  s3->subscribedEventMaskAdd(3);

  {
    InSequence s;
    ExpectCallback1(1, [this](auto) { s1->pendingEventMaskAdd(2); });
    ExpectCallback2(1, [this](auto) { s2->pendingEventMaskRemove(1); });
    ExpectCallback3(1, [this](auto) { s3->pendingEventMaskRemove(3); });
    ExpectCallback1(3);
  }

  ss.dispatch();
  ss.dispatch();
}

TEST_F(SocketTest, UpdatePendingOfNextInline) {
  ScheduleAll();

  {
    InSequence s;
    ExpectCallback1(1, [this](auto) {
      s2->pendingEventMaskAdd(2);
      s3->pendingEventMaskRemove(1);
    });
    ExpectCallback2(1);
    ExpectCallback3(1);
  }

  ss.dispatch();
}

TEST_F(SocketTest, DestroyThisInline) {
  ScheduleAll();

  {
    InSequence s;
    ExpectCallback1(1);
    ExpectCallback2(1, [this](auto) {
      // Callback destroys the socket it belongs to so it won't run again.
      s2->destroy();
    });
    ExpectCallback3(1);
    ExpectCallback1(1);
    ExpectCallback3(1);
  }

  ss.dispatch();
  ss.dispatch();
}

TEST_F(SocketTest, DestroyPrevAndNextInline) {
  ScheduleAll();

  {
    InSequence s;
    ExpectCallback1(1);
    ExpectCallback2(1, [this](auto) {
      s1->destroy();
      s3->destroy();
    });
    ExpectCallback2(1);
  }

  ss.dispatch();
  ss.dispatch();
}

TEST_F(SocketTest, OnFramePoppedDestroyThisInline) {
  Queue q{6};

  PushFrame(*s1, "a");
  PushFrame(*s1, "b");
  PushFrame(*s2, "c");
  PushFrame(*s2, "d");
  PushFrame(*s3, "e");
  PushFrame(*s3, "f");

  {
    InSequence s;
    EXPECT_CALL(*s1, onFramePopped());
    EXPECT_CALL(*s2, onFramePopped()).WillOnce(Invoke(s2, &Socket::destroy));
    EXPECT_CALL(*s3, onFramePopped());
    EXPECT_CALL(*s1, onFramePopped());
    EXPECT_CALL(*s3, onFramePopped());
  }

  ss.drainRoundRobin(q);
  ASSERT_EQ(*q.pop(), "a");
  ASSERT_EQ(*q.pop(), "c");
  ASSERT_EQ(*q.pop(), "e");
  ASSERT_EQ(*q.pop(), "b");
  ASSERT_EQ(*q.pop(), "f");
  ASSERT_FALSE(q.peek());
}

TEST_F(SocketTest, OnFramePoppedDestroyPrevAndNextInline) {
  Queue q{6};

  PushFrame(*s1, "a");
  PushFrame(*s1, "b");
  PushFrame(*s2, "c");
  PushFrame(*s2, "d");
  PushFrame(*s3, "e");
  PushFrame(*s3, "f");

  {
    InSequence s;
    EXPECT_CALL(*s1, onFramePopped());
    EXPECT_CALL(*s2, onFramePopped()).WillOnce(Invoke([this]() {
      s1->destroy();
      s3->destroy();
    }));

    EXPECT_CALL(*s2, onFramePopped());
  }

  ss.drainRoundRobin(q);
  ASSERT_EQ(*q.pop(), "a");
  ASSERT_EQ(*q.pop(), "c");
  ASSERT_EQ(*q.pop(), "d");
  ASSERT_FALSE(q.peek());
}

TEST_F(SocketTest, OnFramedPoppedException) {
  Queue q{4};

  PushFrame(*s1, "a");
  PushFrame(*s1, "b");
  PushFrame(*s2, "c");
  PushFrame(*s3, "d");

  {
    InSequence s;
    EXPECT_CALL(*s1, onFramePopped()).WillOnce(Throw(std::exception{}));
    EXPECT_CALL(*s1, onFramePopped());
  }

  ASSERT_THROW(ss.drainRoundRobin(q), std::exception);
  ASSERT_FALSE(q.peek());

  ss.drainRoundRobin(q);
  ASSERT_EQ(*q.pop(), "c");
  ASSERT_EQ(*q.pop(), "d");
  ASSERT_EQ(*q.pop(), "b");
  ASSERT_FALSE(q.peek());
}

TEST_F(SocketTest, RoundRobinDrain) {
  Queue q{4};
  PushFrame(*s1, "a");
  PushFrame(*s1, "b");
  PushFrame(*s1, "c");
  PushFrame(*s2, "d");
  PushFrame(*s3, "e");
  PushFrame(*s3, "f");
  PushFrame(*s3, "g");
  PushFrame(*s3, "h");

  ss.drainRoundRobin(q);

  ASSERT_FALSE(q.hasCapacity());
  ASSERT_EQ(*q.pop(), "a");
  ASSERT_EQ(*q.pop(), "d");
  ASSERT_EQ(*q.pop(), "e");
  ASSERT_EQ(*q.pop(), "b");

  ss.drainRoundRobin(q);
  ASSERT_EQ(*q.pop(), "f");
  ASSERT_EQ(*q.pop(), "c");
  ASSERT_EQ(*q.pop(), "g");
  ASSERT_EQ(*q.pop(), "h");

  ss.drainRoundRobin(q);
  ASSERT_FALSE(q.peek());
  ASSERT_FALSE(s1->popFrame());
  ASSERT_FALSE(s2->popFrame());
  ASSERT_FALSE(s3->popFrame());
}

}  // namespace detail
}  // namespace unet
