#include <unet/timer.hpp>

#include <boost/assert.hpp>

namespace unet {

void Timer::cancel() {
  core_.unlink();
}

std::chrono::steady_clock::time_point Timer::now() const {
  return core_.manager.now_;
}

void Timer::runAt(std::chrono::steady_clock::time_point tp) {
  cancel();
  core_.runAt = tp;
  core_.manager.cores_.insert(core_);
}

bool Timer::Core::operator<(const Timer::Core& other) const {
  if (runAt < other.runAt) {
    return true;
  }

  // Use address as tie breaker to support more than one timer with the same
  // expiration.
  return this < &other;
}

TimerManager::TimerManager() : TimerManager{std::chrono::steady_clock::now()} {}

TimerManager::TimerManager(std::chrono::steady_clock::time_point now)
    : now_{now} {}

void TimerManager::run(std::chrono::steady_clock::time_point now) {
  BOOST_ASSERT(now >= now_);

  now_ = now;

  auto core = cores_.begin();
  while (!cores_.empty() && (core = cores_.begin())->runAt < now) {
    core->unlink();
    core->f();
  }
}

}  // namespace unet
