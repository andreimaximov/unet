#include <gtest/gtest.h>

#include <unet/detail/list.hpp>

namespace unet {
namespace detail {

namespace {

struct S {
  S() : h{this} {}
  Hook<S> h;
  char x;
};

}  // namespace

TEST(ListTest, Hook) {
  S s;
  ASSERT_EQ(&s.h->x, &s.x);
}

TEST(ListTest, AutoUnlink) {
  List<S> xs;

  {
    S s;
    xs.push_back(s.h);
    ASSERT_EQ(xs.size(), 1);
  }

  ASSERT_EQ(xs.size(), 0);
}

}  // namespace detail
}  // namespace unet
