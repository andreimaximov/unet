#include <unet/detail/arp_cache.hpp>

namespace unet {
namespace detail {

ArpCache::ArpCache(std::size_t capacity, std::chrono::seconds ttl,
                   std::function<std::chrono::steady_clock::time_point()> now)
    : cache_{capacity}, capacity_{capacity}, ttl_{ttl}, now_{now} {}

void ArpCache::add(Ipv4Addr ipv4Addr, EthernetAddr ethAddr) {
  auto now = now_();
  cache_[ipv4Addr] = Item{ethAddr, now, now + ttl_};
  if (cache_.size() > capacity_) {
    gc(now);
  }
}

boost::optional<EthernetAddr> ArpCache::lookup(Ipv4Addr ipv4Addr) {
  // TODO(amaximov): Add an associative cache the size of a cache line to
  // optimize for small working sets.
  auto p = cache_.find(ipv4Addr);
  auto now = now_();

  if (p == cache_.end()) {
    return boost::none;
  } else if (p->second.expireAt < now) {
    cache_.erase(p);
    return boost::none;
  }

  p->second.usedAt = now;
  return p->second.addr;
}

void ArpCache::gc(std::chrono::steady_clock::time_point now) {
  // Phase 1: Remove ALL expired items.
  auto p = cache_.begin();
  auto leastRecentlyUsed = cache_.end();
  while (p != cache_.end()) {
    if (p->second.expireAt < now) {
      p = cache_.erase(p);
    } else {
      if (leastRecentlyUsed == cache_.end() ||
          p->second.usedAt < leastRecentlyUsed->second.usedAt) {
        leastRecentlyUsed = p;
      }
      p++;
    }
  }

  // Phase 2: Remove just ONE least recently used item if capacity was not
  // increased in Phase 1.
  if (cache_.size() > capacity_ && leastRecentlyUsed != cache_.end()) {
    cache_.erase(leastRecentlyUsed);
  }
}

}  // namespace detail
}  // namespace unet
