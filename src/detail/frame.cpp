#include <unet/detail/frame.hpp>

#include <cstring>

#include <boost/assert.hpp>

namespace unet {
namespace detail {

Frame::Frame(std::size_t dataLen)
    : dataLen{dataLen}, buf_{std::make_unique<std::uint8_t[]>(dataLen)} {
  data = buf_.get();
}

std::unique_ptr<Frame> Frame::makeUninitialized(std::size_t dataLen) {
  return std::unique_ptr<Frame>{new Frame{dataLen}};
}

std::unique_ptr<Frame> Frame::makeCopy(const Frame& f) {
  auto copy = makeUninitialized(f.dataLen);
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

std::unique_ptr<Frame> Frame::makeBuf(const std::uint8_t* buf,
                                      std::size_t bufLen) {
  auto f = makeUninitialized(bufLen);
  std::memcpy(f->data, buf, bufLen);
  return f;
}

std::unique_ptr<Frame> Frame::makeStr(const std::string& s) {
  return makeBuf(reinterpret_cast<const std::uint8_t*>(s.data()), s.size());
}

bool Frame::operator==(const std::string& data) const {
  return (dataLen == data.size()) &&
         (std::memcmp(this->data,
                      reinterpret_cast<const std::uint8_t*>(data.data()),
                      dataLen) == 0);
}

}  // namespace detail
}  // namespace unet
