#include <unet/detail/queue.hpp>

#include <boost/assert.hpp>

#include <unet/exception.hpp>

namespace unet {
namespace detail {

static std::size_t capacityOfFrame(const Frame& f, Queue::Policy policy) {
  switch (policy) {
    case Queue::Policy::One:
      return 1;
    case Queue::Policy::DataLen:
      return f.dataLen;
    case Queue::Policy::NetLen:
      return f.netLen;
    case Queue::Policy::TransportLen:
      return f.transportLen;
  }

  throw Exception{"Unknown queue policy!"};
}

Queue::Queue(std::size_t capacity, Policy policy)
    : capacity_{(capacity > 0) ? capacity : 1}, policy_{policy} {}

Queue::~Queue() {
  // Avoid stack overflow when destroying huge queues!
  while (head_) head_ = std::move(head_->next_);
}

boost::optional<Frame&> Queue::peek() {
  return head_ ? boost::optional<Frame&>{*head_} : boost::none;
}

std::unique_ptr<Frame> Queue::pop() {
  auto f = std::move(head_);
  if (!f) {
    return {};
  }

  head_ = std::move(f->next_);
  if (!head_) {
    tail_ = nullptr;
  }

  capacity_ += f->capacity_;
  return f;
}

void Queue::push(std::unique_ptr<Frame>& f) {
  BOOST_ASSERT(f);
  BOOST_ASSERT(!f->next_);

  f->capacity_ = capacityOfFrame(*f, policy_);
  if (capacity_ < f->capacity_) {
    return;
  }

  capacity_ -= f->capacity_;
  auto p = f.get();
  if (tail_) {
    tail_->next_ = std::move(f);
  } else {
    head_ = std::move(f);
  }

  tail_ = p;
}

bool Queue::hasCapacity(std::size_t capacity) const {
  return capacity_ >= capacity;
}

bool Queue::hasCapacity(const Frame& f) const {
  return hasCapacity(capacityOfFrame(f, policy_));
}

}  // namespace detail
}  // namespace unet
