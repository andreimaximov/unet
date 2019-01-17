#include <unet/exception.hpp>

#include <errno.h>
#include <string.h>
#include <cstddef>

namespace unet {

static const char* errnoToStr(char* buf, std::size_t bufLen) {
#ifdef _GNU_SOURCE
  return strerror_r(errno, buf, bufLen);
#else
  strerror_r(errno, buf, bufLen);
  return buf;
#endif
}

Exception::Exception(const char* message) : message_{message} {}

const char* Exception::what() const noexcept {
  return message_;
}

Exception Exception::fromErrNo() {
  Exception ex{nullptr};
  ex.message_ = errnoToStr(ex.buf_, sizeof(ex.buf_));
  return ex;
}

}  // namespace unet
