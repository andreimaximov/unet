#pragma once

#include <exception>

namespace unet {

class Exception : public std::exception {
 public:
  // Creates an exception which borrows the message. This means the message MUST
  // be static data to ensure it stays alive.
  Exception(const char* message);

  const char* what() const noexcept override;

  // Return an exception with a message based on errno.
  static Exception fromErrNo();

 private:
  const char* message_ = nullptr;
  char buf_[64];
};

}  // namespace unet
