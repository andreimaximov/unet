#include <gtest/gtest.h>

#include <unet/stack.hpp>

namespace unet {

TEST(StackTest, RunLoopOnce) {
  Stack stack;
  stack.runLoopOnce();
}

}  // namespace unet
