#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <unet/timer.hpp>

namespace unet {

using testing::InSequence;
using testing::Invoke;
using testing::MockFunction;

static const std::chrono::steady_clock::time_point kTpNowBase{};

TEST(TimerTest, RunAfter) {
  TimerManager manager{kTpNowBase};

  MockFunction<void()> f;
  MockFunction<void()> g;

  {
    InSequence s;
    EXPECT_CALL(g, Call());
    EXPECT_CALL(f, Call());
    EXPECT_CALL(g, Call());
  }

  Timer timer{manager, f.AsStdFunction()};
  timer.runAfter(std::chrono::seconds{1});

  manager.run(kTpNowBase + std::chrono::seconds{1});
  g.Call();
  manager.run(kTpNowBase + std::chrono::seconds{2});
  g.Call();
}

TEST(TimerTest, RunAfterInCallback) {
  TimerManager manager{kTpNowBase};
  Timer* p;

  MockFunction<void()> f;
  MockFunction<void()> g;

  {
    InSequence s;
    EXPECT_CALL(f, Call()).WillOnce(Invoke([&p]() {
      // Make sure an infinite loop does not happen...
      p->runAfter(std::chrono::seconds{0});
    }));
    EXPECT_CALL(g, Call());
    EXPECT_CALL(f, Call());
  }

  Timer timer{manager, f.AsStdFunction()};
  timer.runAfter(std::chrono::seconds{0});
  p = &timer;

  manager.run(kTpNowBase + std::chrono::seconds{1});
  g.Call();
  manager.run(kTpNowBase + std::chrono::seconds{2});
}

TEST(TimerTest, RunAfterMultipleWithSameDelay) {
  TimerManager manager{kTpNowBase};

  MockFunction<void()> f;
  MockFunction<void()> g;

  EXPECT_CALL(f, Call()).Times(1);
  EXPECT_CALL(g, Call()).Times(1);

  Timer timerf{manager, f.AsStdFunction()};
  timerf.runAfter(std::chrono::seconds{0});

  Timer timerg{manager, g.AsStdFunction()};
  timerg.runAfter(std::chrono::seconds{0});

  manager.run(kTpNowBase + std::chrono::seconds{1});
}

TEST(TimerTest, DropTimer) {
  TimerManager manager{kTpNowBase};

  MockFunction<void()> f;
  EXPECT_CALL(f, Call()).Times(1);

  {
    Timer timer{manager, f.AsStdFunction()};
    timer.runAfter(std::chrono::seconds{0});
    manager.run(kTpNowBase + std::chrono::seconds{1});
  }

  manager.run(kTpNowBase + std::chrono::seconds{2});
}

TEST(TimerTest, DropTimerInCallback) {
  TimerManager manager{kTpNowBase};

  std::shared_ptr<Timer> timer;
  timer = std::make_shared<Timer>(manager, [&timer]() { timer.reset(); });
  timer->runAfter(std::chrono::seconds{0});

  manager.run(kTpNowBase + std::chrono::seconds{1});
}

TEST(TimerTest, Now) {
  TimerManager manager{kTpNowBase};
  ASSERT_EQ(manager.now(), kTpNowBase);
  manager.run(kTpNowBase + std::chrono::seconds{1});
  ASSERT_EQ(manager.now(), kTpNowBase + std::chrono::seconds{1});
}

}  // namespace unet
