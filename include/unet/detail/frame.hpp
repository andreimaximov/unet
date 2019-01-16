#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

#include <boost/utility/string_view.hpp>

#include <unet/detail/nonmovable.hpp>

namespace unet {
namespace detail {

class Frame : public NonMovable {
 public:
  std::uint8_t* data = nullptr;
  std::size_t dataLen = 0;

  // Return a frame w/the specified data length. The data is NOT initialized.
  static std::unique_ptr<Frame> make(std::size_t dataLen);

  // Return a frame w/the specified data.
  static std::unique_ptr<Frame> makeStr(boost::string_view data);

  bool operator==(boost::string_view data) const;

 private:
  Frame(std::size_t dataLen);

  // TODO(amaximov): Benchmark and consider variable sized struct allocation.
  std::unique_ptr<Frame> next_;
  std::unique_ptr<std::uint8_t[]> buf_;

  friend class Queue;
};

}  // namespace detail
}  // namespace unet
