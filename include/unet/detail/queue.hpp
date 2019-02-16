#pragma once

#include <cstddef>
#include <memory>

#include <boost/optional.hpp>

#include <unet/detail/frame.hpp>
#include <unet/detail/nonmovable.hpp>

namespace unet {
namespace detail {

// A FIFO queue of frames. All operations are allocation free.
class Queue : public NonMovable {
 public:
  // A policy which determines how much capacity a frame uses.
  enum class Policy {
    One,
    DataLen,
    NetLen,
    TransportLen,
  };

  // Creates a queue that can store up to capacity frames. The policy determines
  // how much capacity a frame uses.
  Queue(std::size_t capacity, Policy policy = Policy::One);

  ~Queue();

  // Return a reference to the head of the queue if the queue is not empty.
  boost::optional<Frame&> peek();

  // Return the removed head of the queue.
  std::unique_ptr<Frame> pop();

  // Pushes a frame f to the end of the queue. You can check if f was moved
  // to find out if the push succeeded. The push can fail if the queue is at
  // its capacity limit.
  void push(std::unique_ptr<Frame>& f);

  // Return true if the queue has space for more frames (up to the specified
  // capacity) and false otherwise.
  bool hasCapacity(std::size_t capacity = 1) const;

  // Return true if the queue has space for f and false otherwise.
  bool hasCapacity(const Frame& f) const;

 private:
  std::size_t capacity_;
  Policy policy_;
  std::unique_ptr<Frame> head_;
  Frame* tail_ = nullptr;
};

}  // namespace detail
}  // namespace unet
