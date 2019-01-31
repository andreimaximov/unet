#include <unet/detail/arp_queue.hpp>

#include <boost/assert.hpp>

namespace unet {
namespace detail {

template <typename F>
static void scanQueue(Queue& q, F&& f) {
  // The expected head of the queue after looping through all frames.
  Frame* nextHead = nullptr;
  while (auto frame = q.pop()) {
    // Already encounter this frame previously, just did not dispose.
    if (frame.get() == nextHead) {
      return;
    }

    f(frame);

    if (frame) {
      nextHead = !nextHead ? frame.get() : nextHead;
      q.push(frame);
    }
  }
}

ArpQueue::ArpQueue(std::size_t delayQueueLen, std::size_t cacheCapacity,
                   std::chrono::seconds delayTimeout,
                   std::chrono::seconds cacheTTL,
                   std::shared_ptr<Queue> sendQueue,
                   std::shared_ptr<TimerManager> timerManager)
    : delayQueue_{delayQueueLen},
      delayTimeout_{delayTimeout},
      cache_{cacheCapacity, cacheTTL,
             [timerManager]() { return timerManager->now(); }},
      sendQueue_{sendQueue},
      timerManager_{timerManager} {}

void ArpQueue::add(Ipv4Addr hopAddr, EthernetAddr ethAddr) {
  cache_.add(hopAddr, ethAddr);

  // Avoid the loop below if we not for sure there are no timers and thus no
  // delayed frames waiting on this hopAddr to be resolved.
  if (timers_.erase(hopAddr) == 0) {
    return;
  }

  // Scan all delayed frames and forward those with the resolved hopAddr.
  scanQueue(delayQueue_, [this, hopAddr, ethAddr](std::unique_ptr<Frame>& f) {
    if (f->hopAddr == hopAddr) {
      f->dataAs<EthernetHeader>()->dstAddr = ethAddr;
      sendQueue_->push(f);

      // Make sure f is dropped in case sendQueue_ is at maximum capacity.
      f.reset();
    }
  });
}

boost::optional<EthernetAddr> ArpQueue::lookup(Ipv4Addr hopAddr) {
  return cache_.lookup(hopAddr);
}

bool ArpQueue::delay(std::unique_ptr<Frame> frame) {
  if (!frame || !delayQueue_.hasCapacity()) {
    return false;
  }

  auto needTimer = (timers_.count(frame->hopAddr) == 0);
  if (needTimer) {
    scheduleTimeout(frame->hopAddr);
  }

  delayQueue_.push(frame);
  return needTimer;
}

void ArpQueue::scheduleTimeout(Ipv4Addr hopAddr) {
  auto timer = std::make_unique<Timer>(*timerManager_, [this, hopAddr]() {
    scanQueue(delayQueue_, [hopAddr](std::unique_ptr<Frame>& f) {
      if (f->hopAddr == hopAddr) {
        f.reset();
      }
    });

    // TODO(amaximov): Make it safe for timers to destroys themselves from the
    // callback.
    timers_.erase(hopAddr);
  });

  timer->runAfter(delayTimeout_);
  timers_.emplace(hopAddr, std::move(timer));
}

}  // namespace detail
}  // namespace unet
