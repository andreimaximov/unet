#include <unet/detail/frame.hpp>

#include <cstring>

#include <boost/assert.hpp>

namespace unet {
namespace detail {

Frame::Frame(std::size_t dataLen)
    : dataLen{dataLen}, buf_{std::make_unique<std::uint8_t[]>(dataLen)} {
  data = buf_.get();
}

std::unique_ptr<Frame> Frame::make(std::size_t dataLen) {
  return std::unique_ptr<Frame>{new Frame{dataLen}};
}

std::unique_ptr<Frame> Frame::makeZeros(std::size_t dataLen) {
  auto f = Frame::make(dataLen);
  std::memset(f->data, 0, dataLen);
  return f;
}

std::unique_ptr<Frame> Frame::make(const Frame& f) {
  auto copy = make(f.dataLen);
  std::memcpy(copy->data, f.data, f.dataLen);
  if (f.net) {
    BOOST_ASSERT(f.net >= f.data);
    copy->net = copy->data + (f.net - f.data);
    copy->netLen = f.netLen;
  }
  if (f.transport) {
    BOOST_ASSERT(f.transport >= f.data);
    copy->transport = copy->data + (f.transport - f.data);
    copy->transportLen = f.transportLen;
  }
  return copy;
}

std::unique_ptr<Frame> Frame::makeStr(boost::string_view data) {
  auto f = make(data.size());
  std::memcpy(f->data, data.data(), data.size());
  return f;
}

bool Frame::operator==(boost::string_view data) const {
  return (dataLen == data.size()) &&
         (std::memcmp(this->data,
                      reinterpret_cast<const std::uint8_t*>(data.data()),
                      dataLen) == 0);
}

}  // namespace detail
}  // namespace unet
