#include <unet/event.hpp>

namespace unet {

std::uint32_t operator|(std::uint32_t lhs, Event rhs) {
  return lhs | detail::eventAsInt(rhs);
}

std::uint32_t operator|(Event lhs, Event rhs) {
  return detail::eventAsInt(lhs) | rhs;
}

std::uint32_t operator&(std::uint32_t lhs, Event rhs) {
  return lhs & detail::eventAsInt(rhs);
}

std::uint32_t operator&(Event lhs, Event rhs) {
  return detail::eventAsInt(lhs) & rhs;
}

namespace detail {

std::uint32_t eventAsInt(Event ev) {
  return static_cast<std::uint32_t>(ev);
}

}  // namespace detail
}  // namespace unet
