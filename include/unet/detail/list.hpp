#pragma once

#include <boost/assert.hpp>
#include <boost/intrusive/list.hpp>

#include <unet/detail/nonmovable.hpp>

namespace unet {
namespace detail {

template <typename T>
class Hook : public NonMovable,
             public boost::intrusive::list_base_hook<
                 boost::intrusive::link_mode<boost::intrusive::auto_unlink>> {
 public:
  Hook(T* x) : x_{x} {}

  Hook() = default;

  T* operator->() const {
    BOOST_ASSERT(x_);
    return x_;
  }

  T& operator*() const {
    BOOST_ASSERT(x_);
    return *x_;
  }

 private:
  T* x_ = nullptr;
};

template <typename T>
using List =
    boost::intrusive::list<Hook<T>,
                           boost::intrusive::constant_time_size<false>>;

}  // namespace detail
}  // namespace unet
