#include <benchmark/benchmark.h>

#include <unet/detail/socket.hpp>
#include <unet/detail/socket_set.hpp>

namespace unet {
namespace detail {

constexpr auto kNumSockets = 100'000;

static void benchSocketSetDispatch(benchmark::State& state) {
  SocketSet ss;

  for (int i = 0; i < kNumSockets; i++) {
    auto s = new Socket{ss, 1, Queue::Policy::One, [](auto) {}};
    s->subscribedEventMaskAdd(1);
    s->pendingEventMaskAdd(1);
  }

  for (auto _ : state) {
    ss.dispatch();
  }
}

static void benchSocketSetDrainRoundRobin(benchmark::State& state) {
  for (auto _ : state) {
    state.PauseTiming();

    SocketSet ss;
    auto q = std::make_unique<Queue>(kNumSockets);

    for (int i = 0; i < kNumSockets; i++) {
      auto s = new Socket{ss, 1, Queue::Policy::One, [](auto) {}};
      auto f = Frame::makeStr("What a beautiful frame...");
      s->sendFrame(f);
    }

    state.ResumeTiming();
    ss.drainRoundRobin(*q);
    state.PauseTiming();

    q = nullptr;

    state.ResumeTiming();
  }
}

BENCHMARK(benchSocketSetDispatch)->Unit(benchmark::kMicrosecond);
BENCHMARK(benchSocketSetDrainRoundRobin)->Unit(benchmark::kMicrosecond);

}  // namespace detail
}  // namespace unet
