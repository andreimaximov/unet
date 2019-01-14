#include <benchmark/benchmark.h>

#include <unet/stack.hpp>

namespace unet {

static void benchStackRunLoopOnce(benchmark::State& state) {
  Stack stack;
  for (auto _ : state) {
    stack.runLoopOnce();
  }
}

BENCHMARK(benchStackRunLoopOnce);

}  // namespace unet
