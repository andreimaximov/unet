#include <unet/detail/serializer.hpp>

namespace unet {
namespace detail {

Serializer::Serializer(EthernetAddr ethAddr, Ipv4Addr ipv4Addr)
    : ethAddr_{ethAddr}, ipv4Addr_{ipv4Addr} {}

}  // namespace detail
}  // namespace unet
