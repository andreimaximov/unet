#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

#include <boost/assert.hpp>

#include <unet/detail/nonmovable.hpp>
#include <unet/wire/ipv4.hpp>

namespace unet {
namespace detail {

class Frame : public NonMovable {
 public:
  std::uint8_t* data = nullptr;
  std::uint8_t* net = nullptr;
  std::uint8_t* transport = nullptr;
  std::size_t dataLen = 0;
  std::size_t netLen = 0;
  std::size_t transportLen = 0;

  // We need this to distinguish between IPv4 frames which we need to perform an
  // Ethernet address lookup and those for which we do not (eg. IPv4 frames
  // crafted from raw Ethernet sockets).
  bool doIpv4Routing = false;

  // The next IPv4 address to send this frame to on the way to its final
  // destination.
  Ipv4Addr hopAddr{};

  // Return a frame w/the specified data length. The data is NOT initialized.
  static std::unique_ptr<Frame> makeUninitialized(std::size_t dataLen);

  // Return a frame w/the copied contents of frame f.
  static std::unique_ptr<Frame> makeCopy(const Frame& f);

  // Return a zerod frame w/the specified data length.
  static std::unique_ptr<Frame> makeZeros(std::size_t dataLen);

  // Return a frame w/the specified data.
  static std::unique_ptr<Frame> makeStr(const std::string& s);

  bool operator==(const std::string& data) const;

  template <typename T>
  T* dataAs() {
    return bufAs<T>(data, dataLen);
  }

  template <typename T>
  T* netAs() {
    return bufAs<T>(net, netLen);
  }

  template <typename T>
  T* transportAs() {
    return bufAs<T>(transport, transportLen);
  }

 private:
  Frame(std::size_t dataLen);

  template <typename T>
  T* bufAs(std::uint8_t* p, std::size_t len) {
    BOOST_ASSERT(p >= buf_.get());
    BOOST_ASSERT(len >= sizeof(T));
    return reinterpret_cast<T*>(p);
  }

  // TODO(amaximov): Benchmark and consider variable sized struct allocation.
  std::unique_ptr<Frame> next_;
  std::unique_ptr<std::uint8_t[]> buf_;
  std::size_t capacity_;

  friend class Queue;
};

}  // namespace detail
}  // namespace unet
