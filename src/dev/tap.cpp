#include <unet/dev/tap.hpp>

#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef __linux__
#include <linux/if.h>
#include <linux/if_tun.h>
#endif

#include <cstring>

#include <unet/exception.hpp>

namespace unet {

Tap::Tap(const std::string& name) {
#ifdef __linux__
  if (name.size() >= IFNAMSIZ) {
    throw Exception{"Interface name should be < IFNAMSIZ."};
  }

  // Open TAP...
  if ((fd_ = open("/dev/net/tun", O_RDWR | O_NONBLOCK)) == -1) {
    throw Exception::fromErrNo();
  }

  ifreq ifr;
  std::strncpy(ifr.ifr_name, name.data(), IFNAMSIZ);
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
  if (ioctl(fd_, TUNSETIFF, &ifr) == -1) {
    close(fd_);
    throw Exception::fromErrNo();
  }

  // Query MTU...
  auto sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd == -1) {
    close(fd_);
    throw Exception::fromErrNo();
  }

  if (ioctl(sockfd, SIOCGIFMTU, &ifr) == -1) {
    close(fd_);
    close(sockfd);
    throw Exception::fromErrNo();
  } else if (ifr.ifr_mtu == 0) {
    close(fd_);
    close(sockfd);
    throw Exception{"Tap cannot have an MTU of 0."};
  }

  maxTransmissionUnit_ = ifr.ifr_mtu;
  close(sockfd);

#else
  (void)name;
  throw Exception{"Tap devices are supported only on Linux."};
#endif
}

Tap::~Tap() {
  if (fd_ != 0) {
    close(fd_);
  }
}

std::size_t Tap::send(const std::uint8_t* buf, std::size_t bufLen) {
  if (!buf || !bufLen) {
    return 0;
  }

  auto w = write(fd_, buf, bufLen);
  if (w < 0 && errno == EAGAIN) {
    return 0;
  } else if (w < 0) {
    throw Exception::fromErrNo();
  } else {
    return w;
  }
}

std::size_t Tap::read(std::uint8_t* buf, std::size_t bufLen) {
  if (!buf || !bufLen) {
    return 0;
  }

  auto r = ::read(fd_, buf, bufLen);
  if (r < 0 && errno == EAGAIN) {
    return 0;
  } else if (r < 0) {
    throw Exception::fromErrNo();
  } else {
    return r;
  }
}

std::size_t Tap::maxTransmissionUnit() const {
  return maxTransmissionUnit_;
}

}  // namespace unet
