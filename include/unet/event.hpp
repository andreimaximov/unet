#pragma once

#include <cstdint>

namespace unet {

// A set of events indicating the state of a socket. Sockets offer a
// subscription API to invoke a callback when a socket becomes sendable,
// readable, etc.
enum class Event : std::uint32_t {
  // Indicates a socket is ready to send data.
  Send = 1,
  // Indicates a socket is ready to read data.
  Read = 2,
};

std::uint32_t operator|(std::uint32_t lhs, Event rhs);
std::uint32_t operator|(Event lhs, Event rhs);
std::uint32_t operator&(std::uint32_t lhs, Event rhs);
std::uint32_t operator&(Event lhs, Event rhs);

namespace detail {

std::uint32_t eventAsInt(Event ev);

}

}  // namespace unet
