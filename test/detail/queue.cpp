#include <gtest/gtest.h>

#include <unet/detail/queue.hpp>

namespace unet {
namespace detail {

TEST(QueueTest, PushPeekAndPop) {
  Queue q{2};

  auto f1 = Frame::makeStr("a");
  q.push(f1);
  ASSERT_FALSE(f1);

  auto f2 = Frame::makeStr("b");
  q.push(f2);
  ASSERT_FALSE(f2);

  auto f3 = q.peek();
  ASSERT_TRUE(f3);
  ASSERT_EQ(*f3, "a");

  auto f4 = q.pop();
  ASSERT_TRUE(f4);
  ASSERT_EQ(*f4, "a");

  auto f5 = q.peek();
  ASSERT_TRUE(f5);
  ASSERT_EQ(*f5, "b");

  auto f6 = q.pop();
  ASSERT_TRUE(f6);
  ASSERT_EQ(*f6, "b");
}

TEST(QueueTest, PushFull) {
  Queue q{1};

  auto f1 = Frame::makeStr("a");
  q.push(f1);
  ASSERT_FALSE(f1);

  auto f2 = Frame::makeStr("b");
  q.push(f2);
  ASSERT_TRUE(f2);
}

TEST(QueueTest, PeekPopEmpty) {
  Queue q{1};

  auto f1 = q.peek();
  ASSERT_FALSE(f1);

  auto f2 = q.pop();
  ASSERT_FALSE(f2);
}

TEST(QueueTest, PopAddsCapacity) {
  Queue q{1};

  auto f1 = Frame::makeStr("a");
  q.push(f1);
  ASSERT_FALSE(f1);

  auto f2 = q.pop();
  ASSERT_TRUE(f2);
  ASSERT_EQ(*f2, "a");

  auto f3 = Frame::makeStr("b");
  q.push(f3);
  ASSERT_FALSE(f3);

  auto f4 = q.pop();
  ASSERT_TRUE(f4);
  ASSERT_EQ(*f4, "b");
}

TEST(QueueTest, HasCapacity) {
  Queue q{1};
  ASSERT_TRUE(q.hasCapacity());

  auto f = Frame::makeStr("a");
  q.push(f);
  ASSERT_FALSE(q.hasCapacity());

  q.pop();
  ASSERT_TRUE(q.hasCapacity());
}

TEST(QueueTest, DestroyHuge) {
  Queue q{100'000};

  for (auto count = 0; count < 100'000; count++) {
    auto f = Frame::makeUninitialized(1);
    q.push(f);
    ASSERT_FALSE(f);
  }
}

TEST(QueueTest, NonDefaultPolicy) {
  auto f1 = Frame::makeStr("a");
  auto f2 = Frame::makeStr("ab");
  auto f3 = Frame::makeStr("abc");
  auto p1 = f1.get();
  auto p2 = f2.get();
  auto p3 = f3.get();

  Queue q{4, Queue::Policy::DataLen};

  q.push(f1);
  ASSERT_FALSE(f1);

  q.push(f2);
  ASSERT_FALSE(f2);

  q.push(f3);
  ASSERT_TRUE(f3);

  auto f4 = q.pop();
  ASSERT_EQ(f4.get(), p1);

  auto f5 = q.pop();
  ASSERT_EQ(f5.get(), p2);

  q.push(f3);
  ASSERT_FALSE(f3);

  auto f6 = q.pop();
  ASSERT_EQ(f6.get(), p3);
}

}  // namespace detail
}  // namespace unet
