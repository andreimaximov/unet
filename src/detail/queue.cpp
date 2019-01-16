#include <unet/detail/queue.hpp>

#include <boost/assert.hpp>

namespace unet {
namespace detail {

Queue::Queue(std::size_t capacity) : capacity_{(capacity > 0) ? capacity : 1} {}

Queue::~Queue() {
  // Avoid stack overflow when destroying huge queues!
  while (head_) head_ = std::move(head_->next_);
}

boost::optional<Frame&> Queue::peek() {
  return head_ ? boost::optional<Frame&>{*head_} : boost::none;
}

std::unique_ptr<Frame> Queue::pop() {
  auto f = std::move(head_);
  if (!f) return f;
  head_ = std::move(f->next_);
  if (!head_) tail_ = nullptr;
  capacity_++;
  return f;
}

void Queue::push(std::unique_ptr<Frame>& f) {
  BOOST_ASSERT(f);
  BOOST_ASSERT(!f->next_);

  if (capacity_ == 0) return;
  capacity_--;
  auto p = f.get();
  if (tail_) {
    tail_->next_ = std::move(f);
  } else {
    head_ = std::move(f);
  }
  tail_ = p;
}

bool Queue::hasCapacity() const {
  return capacity_ > 0;
}

}  // namespace detail
}  // namespace unet
