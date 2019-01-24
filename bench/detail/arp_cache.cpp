#include <cstdint>
#include <cstring>
#include <vector>

#include <benchmark/benchmark.h>

#include <unet/detail/arp_cache.hpp>
#include <unet/random.hpp>

namespace unet {
namespace detail {

constexpr auto kNumAddOps = 65'536;

static Ipv4Addr randomIpv4(std::uint32_t maxAddress) {
  auto raw = randInt<std::uint32_t>(0, maxAddress);
  Ipv4Addr addr;
  std::memcpy(&addr, &raw, sizeof(raw));
  return addr;
}

static std::vector<Ipv4Addr> randomIpv4s(std::uint32_t maxAddress) {
  std::vector<Ipv4Addr> addresses;
  while (addresses.size() < kNumAddOps) {
    addresses.push_back(randomIpv4(maxAddress));
  }
  return addresses;
}

static void benchArpCacheAdd(benchmark::State& state) {
  auto capacity = static_cast<std::size_t>(state.range(0));
  ArpCache cache{capacity, std::chrono::seconds{60}};

  auto maxAddress = static_cast<std::uint32_t>(state.range(1));
  auto addresses = randomIpv4s(maxAddress);

  for (auto _ : state) {
    for (auto addr : addresses) {
      cache.add(addr, EthernetAddr{});
    }
  }
}

static void benchArpCacheLookup(benchmark::State& state) {
  auto capacity = static_cast<std::size_t>(state.range(0));
  ArpCache cache{capacity, std::chrono::seconds{60}};

  auto maxAddress = static_cast<std::uint32_t>(state.range(1));
  auto addresses = randomIpv4s(maxAddress);
  for (auto addr : addresses) {
    cache.add(addr, EthernetAddr{});
  }

  for (auto _ : state) {
    for (auto addr : addresses) {
      benchmark::DoNotOptimize(cache.lookup(addr));
    }
  }
}

BENCHMARK(benchArpCacheAdd)
    ->RangeMultiplier(64)
    ->Ranges({{64, 4'096}, {64, 4'096}})
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(benchArpCacheLookup)
    ->RangeMultiplier(64)
    ->Ranges({{64, 4'096}, {64, 4'096}})
    ->Unit(benchmark::kMicrosecond);

}  // namespace detail
}  // namespace unet
