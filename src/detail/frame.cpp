#include <unet/detail/frame.hpp>

#include <algorithm>

namespace unet {
namespace detail {

Frame::Frame(std::size_t dataLen)
    : dataLen{dataLen}, buf_{std::make_unique<std::uint8_t[]>(dataLen)} {
  data = buf_.get();
}

std::unique_ptr<Frame> Frame::make(std::size_t dataLen) {
  return std::unique_ptr<Frame>{new Frame{dataLen}};
}

std::unique_ptr<Frame> Frame::makeStr(boost::string_view data) {
  auto f = make(data.size());
  std::copy(data.begin(), data.end(), f->data);
  return f;
}

bool Frame::operator==(boost::string_view data) const {
  return (dataLen == data.size()) &&
         std::equal(this->data, this->data + dataLen,
                    reinterpret_cast<const std::uint8_t*>(data.data()));
}

}  // namespace detail
}  // namespace unet
