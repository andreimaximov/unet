#pragma once

#include <string>

#include <unet/detail/nonmovable.hpp>
#include <unet/dev/dev.hpp>

namespace unet {

class Tap : public Dev, public detail::NonMovable {
 public:
  // Creates a Linux TAP interface w/the provided name.
  Tap(const std::string& name);
  ~Tap();

  std::size_t send(const std::uint8_t* buf, std::size_t bufLen) override;
  std::size_t read(std::uint8_t* buf, std::size_t bufLen) override;
  std::size_t maxTransmissionUnit() const override;

 private:
  int fd_ = 0;
  std::size_t maxTransmissionUnit_ = 0;
};

}  // namespace unet
