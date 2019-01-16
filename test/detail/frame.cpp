#include <gtest/gtest.h>

#include <unet/detail/frame.hpp>

namespace unet {
namespace detail {

TEST(FrameTest, Make) {
  auto f = Frame::make(1024);
  ASSERT_EQ(f->dataLen, 1024);
}

}  // namespace detail
}  // namespace unet
