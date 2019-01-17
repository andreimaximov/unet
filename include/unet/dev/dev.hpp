#pragma once

#include <cstddef>
#include <cstdint>

namespace unet {

// A device for sending and receiving raw frames.
class Dev {
 public:
  virtual ~Dev() = default;

  // Sends a frame from the buffer across the link.
  //
  // Return the number of bytes sent or throws an Exception in case of an error.
  // A return of 0 for a non-zero length frame indicates the device is
  // exhausted.
  virtual std::size_t send(const std::uint8_t* buf, std::size_t bufLen) = 0;

  // Reads a frame from the link to the buffer.
  //
  // Return the number of bytes read or throws an Exception in case of an error.
  // A return of 0 indicates the device is exhausted.
  virtual std::size_t read(std::uint8_t* buf, std::size_t bufLen) = 0;

  // Return the Max Transmission Unit of this device. This should never under
  // any circumstances return 0.
  virtual std::size_t maxTransmissionUnit() const = 0;
};

}  // namespace unet
