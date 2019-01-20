#include <gtest/gtest.h>

#include <unet/wire/wire.hpp>

namespace unet {

namespace {

struct S {
  struct MemcmpTag {};
  int bar;
};

}  // namespace

TEST(WireTest, Memcmp) {
  S x{0};
  S y{0};
  S z{1};

  // GTest has issues w/the MemcmpTag based operator==(...) when used w/a struct
  // in an anonymous namespace...
  ASSERT_TRUE(x == y);
  ASSERT_FALSE(x == z);
  ASSERT_FALSE(y == z);
}

}  // namespace unet
