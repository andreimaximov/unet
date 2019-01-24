#include <gtest/gtest.h>

#include <unet/detail/arp_cache.hpp>

namespace unet {
namespace detail {

constexpr Ipv4Addr kIpv4[] = {Ipv4Addr{0, 1, 2, 3}, Ipv4Addr{1, 2, 3, 4},
                              Ipv4Addr{2, 3, 4, 5}};
constexpr EthernetAddr kEthernet[] = {EthernetAddr{0, 1, 2, 3, 4, 5},
                                      EthernetAddr{1, 2, 3, 4, 5, 6},
                                      EthernetAddr{2, 3, 4, 5, 6, 7}};

class ArpCacheTest : public testing::Test {
 public:
  ArpCache makeArpCache(std::size_t capacity, std::uint64_t ttl) {
    return ArpCache{capacity, std::chrono::seconds{ttl},
                    [this]() { return this->now; }};
  }

  std::chrono::steady_clock::time_point now;
};

TEST_F(ArpCacheTest, LookupAdded) {
  auto cache = makeArpCache(1, 1);
  ASSERT_FALSE(cache.lookup(kIpv4[0]));
  cache.add(kIpv4[0], kEthernet[0]);
  ASSERT_EQ(cache.lookup(kIpv4[0]).value(), kEthernet[0]);
}

TEST_F(ArpCacheTest, LookupExpired) {
  auto cache = makeArpCache(1, 1);
  cache.add(kIpv4[0], kEthernet[0]);
  now += std::chrono::seconds{2};
  ASSERT_FALSE(cache.lookup(kIpv4[0]));
}

TEST_F(ArpCacheTest, GarbageCollectExpired) {
  auto cache = makeArpCache(2, 1);
  cache.add(kIpv4[0], kEthernet[0]);
  now += std::chrono::seconds{1};
  cache.add(kIpv4[1], kEthernet[1]);
  now += std::chrono::seconds{1};
  cache.add(kIpv4[2], kEthernet[2]);
  ASSERT_FALSE(cache.lookup(kIpv4[0]));
  ASSERT_EQ(cache.lookup(kIpv4[1]).value(), kEthernet[1]);
  ASSERT_EQ(cache.lookup(kIpv4[2]).value(), kEthernet[2]);
}

TEST_F(ArpCacheTest, GarbageCollectLru) {
  auto cache = makeArpCache(2, 10);
  cache.add(kIpv4[0], kEthernet[0]);
  cache.add(kIpv4[1], kEthernet[1]);
  now += std::chrono::seconds{1};
  cache.lookup(kIpv4[0]);
  cache.add(kIpv4[2], kEthernet[2]);
  ASSERT_EQ(cache.lookup(kIpv4[0]).value(), kEthernet[0]);
  ASSERT_FALSE(cache.lookup(kIpv4[1]));
  ASSERT_EQ(cache.lookup(kIpv4[2]).value(), kEthernet[2]);
}

}  // namespace detail
}  // namespace unet
