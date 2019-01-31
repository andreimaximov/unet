#include <gtest/gtest.h>

#include <unet/detail/frame.hpp>

namespace unet {
namespace detail {

namespace {

struct S {
  std::uint8_t a;
  std::uint8_t b;
  std::uint8_t c;
  std::uint8_t d;
};

}  // namespace

TEST(FrameTest, Make) {
  auto f = Frame::make(1024);
  ASSERT_EQ(f->dataLen, 1024);
}

TEST(FrameTest, Copy) {
  auto f1 = Frame::makeStr("abc...");
  f1->net = f1->data + 3;
  f1->netLen = f1->dataLen - 3;

  auto f2 = Frame::make(*f1);
  ASSERT_EQ(f2->net, f2->data + 3);
  ASSERT_EQ(f2->netLen, 3);
}

TEST(FrameTest, DataAs) {
  auto f = Frame::make(sizeof(S));
  f->data[0] = 42;
  ASSERT_EQ(f->dataAs<S>()->a, 42);
  f->dataAs<S>()->a = 0;
  ASSERT_EQ(f->data[0], 0);
}

TEST(FrameTest, NetAs) {
  auto f = Frame::make(sizeof(S) * 2);
  f->net = f->data + sizeof(S);
  f->netLen = f->dataLen - sizeof(S);
  f->data[4] = 42;
  ASSERT_EQ(f->netAs<S>()->a, 42);
  f->netAs<S>()->a = 0;
  ASSERT_EQ(f->data[4], 0);
}

}  // namespace detail
}  // namespace unet
