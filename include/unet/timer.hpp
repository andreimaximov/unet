#pragma once

#include <chrono>
#include <functional>

#include <boost/intrusive/set.hpp>

#include <unet/detail/nonmovable.hpp>

namespace unet {

class TimerManager;

// A timer which can be repeatedly scheduled and executes a callback upon
// expiration.
class Timer : public detail::NonMovable {
 public:
  // Creates a timer registered to the provided timer manager. The lifetime of
  // the timer must NOT exceed that of the timer manager. The function f is
  // executed upon expiration and automatically descheduled upon destruction of
  // the timer. It is safe to schedule and/or cancel this or any other timers
  // from within f.
  template <typename F>
  Timer(TimerManager& manager, F&& f) : core_{manager, std::forward<F>(f)} {}

  // Schedules the timer to expire after more than delay time has passed. If
  // already scheduled, it is first cancelled.
  template <typename R, typename P>
  void runAfter(std::chrono::duration<R, P> delay) {
    runAt(now() + delay);
  }

  // Cancels the expiration of the timer.
  void cancel();

 private:
  // Avoid exposing operator<.
  class Core : public boost::intrusive::set_base_hook<
                   boost::intrusive::link_mode<boost::intrusive::auto_unlink>> {
   public:
    template <typename F>
    Core(TimerManager& manager, F&& f)
        : manager{manager}, f{std::forward<F>(f)} {}

    TimerManager& manager;
    std::function<void()> f;
    std::chrono::steady_clock::time_point runAt{};

    bool operator<(const Core& other) const;
  };

  std::chrono::steady_clock::time_point now() const;

  void runAt(std::chrono::steady_clock::time_point tp);

  Core core_;

  friend class TimerManager;
};

// A scheduler for the expiration of timers.
class TimerManager : public detail::NonMovable {
 public:
  // Creates a timer manager deriving now from the steady clock.
  TimerManager();

  // Creates a timer manager deriving now from the provided time point.
  TimerManager(std::chrono::steady_clock::time_point now);

  // Updates the current time and executes callbacks for all expires  It is NOT
  // safe to call this function from a callback (aka recursively).
  void run(std::chrono::steady_clock::time_point now =
               std::chrono::steady_clock::now());

  // Return the last now timestamp passed to the update tick.
  std::chrono::steady_clock::time_point now() const;

 private:
  // TODO(amaximov): Consider using a Hashed Hierarchial Timing Wheel design:
  // http://www.cs.columbia.edu/~nahum/w6998/papers/sosp87-timing-wheels.pdf
  boost::intrusive::set<Timer::Core,
                        boost::intrusive::constant_time_size<false>>
      cores_;
  std::chrono::steady_clock::time_point now_;

  friend class Timer;
};

}  // namespace unet
