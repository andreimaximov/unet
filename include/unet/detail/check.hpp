#pragma once

#include <cstddef>
#include <cstdint>

namespace unet {
namespace detail {

// Calculates the Internet Checksum of buf as defined by RFC1071:
// https://tools.ietf.org/html/rfc1071
std::uint16_t checksum(const std::uint8_t* buf, std::size_t bufLen);

}  // namespace detail
}  // namespace unet
