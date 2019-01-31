#pragma once

#include <cstddef>
#include <cstdint>

#include <unet/detail/frame.hpp>
#include <unet/detail/list.hpp>
#include <unet/detail/queue.hpp>
#include <unet/detail/socket.hpp>
#include <unet/wire/ethernet.hpp>

namespace unet {
namespace detail {

// A socket for communicating via Ethernet or IPv4 frames.
class RawSocket : public Socket {
 public:
  static constexpr std::uint32_t kEthernet = 0;
  static constexpr std::uint32_t kIpv4 = 1;

  RawSocket(std::uint32_t socketType, std::size_t sendQueueLen,
            std::size_t readQueueLen, std::size_t maxTransmissionUnit,
            EthernetAddr ethAddr, List<RawSocket>& sockets,
            SocketSet& socketSet, Callback callback);

  ~RawSocket() override = default;

  void onFramePopped() override;

  void process(const Frame& f);

  std::size_t send(const std::uint8_t* buf, std::size_t bufLen);

  std::size_t read(std::uint8_t* buf, std::size_t bufLen);

  // Closes the socket. The socket will be destroyed once all egress frames have
  // been drained. Thus the socket may or may not be destroyed inline so you
  // should not do anything with it after this call.
  void close();

 private:
  std::uint32_t socketType_;
  Hook<RawSocket> socketsHook_;
  Queue readQueue_;
  std::size_t maxTransmissionUnit_;
  EthernetAddr ethAddr_;
  bool closed_ = false;
};

}  // namespace detail
}  // namespace unet
