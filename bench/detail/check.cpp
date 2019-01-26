#include <cstdint>
#include <vector>

#include <benchmark/benchmark.h>

#include <unet/detail/check.hpp>
#include <unet/random.hpp>

namespace unet {
namespace detail {

static void benchChecksum(benchmark::State& state) {
  std::vector<std::uint8_t> buf(1024);
  for (auto& b : buf) {
    b = randInt<std::uint8_t>();
  }

  for (auto _ : state) {
    benchmark::DoNotOptimize(checksum(buf.data(), buf.size()));
  }
}

BENCHMARK(benchChecksum);

}  // namespace detail
}  // namespace unet
