#pragma once

namespace unet {
namespace detail {

class NonMovable {
 public:
  NonMovable() = default;
  NonMovable(const NonMovable&) = delete;
  NonMovable(NonMovable&&) = delete;
  NonMovable operator=(const NonMovable&) = delete;
  NonMovable operator=(NonMovable&&) = delete;
  ~NonMovable() = default;
};

}  // namespace detail
}  // namespace unet
