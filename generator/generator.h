#pragma once

#include <coroutine>
#include <exception>
#include <utility>
#include <type_traits>
#include <ranges>

namespace tl {
   template <class T>
   class generator;

   namespace detail {
      template <class T>
      struct generator_promise {
         using value_type = std::remove_reference_t<T>;
         using reference_type = value_type&;
         using pointer_type = value_type*;

         generator_promise() = default;

         generator<T> get_return_object();

         std::suspend_always initial_suspend() { return {}; }
         std::suspend_always final_suspend() noexcept { return {}; }

         void return_void() { return; }

         void unhandled_exception() {
            exception_ = std::current_exception();
         }

         void rethrow_if_exception() {
            if (exception_) {
               std::rethrow_exception(exception_);
            }
         }

         std::suspend_always yield_value(reference_type v) {
            value_ = std::addressof(v);
            return {};
         }

         std::exception_ptr exception_;
         pointer_type value_;
      };

      class generator_sentinel {};

      template <class T>
      struct generator_iterator {
         using promise_type = detail::generator_promise<T>;
         using value_type = promise_type::value_type;
         using reference_type = promise_type::reference_type;
         using pointer_type = promise_type::pointer_type;
         using handle_type = std::coroutine_handle<promise_type>;
         using difference_type = std::ptrdiff_t;
         
         generator_iterator() = default;
         //Non-copyable because coroutine handles point to a unique resource
         generator_iterator(generator_iterator const&) = default; //DRAGONS
         generator_iterator(generator_iterator&& rhs) : handle_(std::exchange(rhs.handle_, nullptr)) {}
         generator_iterator& operator=(generator_iterator const&) = default;
         generator_iterator& operator=(generator_iterator&& rhs) {
            handle_ = std::exchange(rhs.handle_, nullptr);
            return *this;
         }

         friend bool operator==(generator_iterator const& it, generator_sentinel) {
            return (!it.handle_ || it.handle_.done());
         }
         friend bool operator!=(generator_iterator const& it, generator_sentinel s) {
            return !(it == s);
         }

         generator_iterator(handle_type handle) : handle_(handle) {}

         generator_iterator& operator++() {
            handle_.resume();
            if (handle_.done()) {
               handle_.promise().rethrow_if_exception();
            }
            return *this;
         }

         void operator++(int) {
            (void)this->operator++();
         }

         reference_type operator*() {
            return *handle_.promise().value_;
         }

         handle_type handle_;
      };
   }

   template <class T>
   class generator {
   public:
      using promise_type = detail::generator_promise<T>;
      using handle_type = std::coroutine_handle<promise_type>;

      generator() = default;
      explicit generator(handle_type handle) : handle_(handle) {}

      generator(generator const&) = delete;
      generator(generator&& rhs) : handle_(std::exchange(rhs.handle_, nullptr)) {}
      generator& operator=(generator const&) = delete;
      generator& operator=(generator&& rhs) {
         handle_ = std::exchange(rhs.handle_, nullptr);
         return *this;
      }

      detail::generator_iterator<T> begin() {
         handle_.resume();
         if (handle_.done()) {
            handle_.promise().rethrow_if_exception();
         }
         return {handle_};
      }

      detail::generator_sentinel end() const {
         return {};
      }

   private:
      handle_type handle_;
   };

   namespace detail {
      template<class T>
      inline generator<T> generator_promise<T>::get_return_object()
      {
         return generator<T>(std::coroutine_handle<generator_promise>::from_promise(*this));
      }
   }
}

template<class T>
inline constexpr bool std::ranges::enable_view<tl::generator<T>> = true;