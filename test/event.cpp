#include <gtest/gtest.h>

#include <unet/event.hpp>

namespace unet {

TEST(EventTest, BitwiseOr) {
  std::uint32_t mask = 0;
  ASSERT_EQ(mask | Event::Send, 1);
  ASSERT_EQ(mask | Event::Read, 2);
  ASSERT_EQ(Event::Send | Event::Read, 3);
}

TEST(EventTest, BitwiseAnd) {
  std::uint32_t mask = 0xffff;
  ASSERT_EQ(mask & Event::Send, 1);
  ASSERT_EQ(mask & Event::Read, 2);
  ASSERT_EQ(Event::Send & Event::Read, 0);
}

}  // namespace unet
