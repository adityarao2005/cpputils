#pragma once
#include <cpputils/cpputils_api.h>
#include <cpputils/core.h>
#include <coroutine>
#include <optional>
#include <iostream>
#include <atomic>
#include <cpputils/asyncio/reset_events.h>
#include <cpputils/asyncio/coroutine_semantics.h>
#include <cpputils/asyncio/sync_wait_task.h>
#include <functional>

namespace cpputils
{
	namespace asyncio
	{
		// Coroutines stuff
		// - Awaitable type: A type that can be used in a co_await expression.
		// - Awaiter type: A type that controls the resumption of a coroutine that has been suspended by a co_await expression.
		// - Coroutine handle: A handle to a coroutine that can be used to resume execution of the coroutine.
		// - Promise type: A type that is used to control the coroutine's execution.
		// - Coroutine type: A type that represents a coroutine.
		// For promise types, for initial suspend, for things that will eagerly execute, use std::suspend_never
		// For things that'll lazily execute i.e on co_await, use std::suspend_always and use noexcept
		// For promise types, for final suspend, it is recommended that the coroutine is suspended to perform the manual cleanup with ".destroy()" in the destructor

		// You can control how co_await works within the coroutine with await_transform
		// For example
		/*
		template<typename T>
class optional_promise
{
  ...

  template<typename U>
  auto await_transform(std::optional<U>& value)
  {
	class awaiter
	{
	  std::optional<U>& value;
	public:
	  explicit awaiter(std::optional<U>& x) noexcept : value(x) {}
	  bool await_ready() noexcept { return value.has_value(); }
	  void await_suspend(std::coroutine_handle<>) noexcept {}
	  U& await_resume() noexcept { return *value; }
	};
	return awaiter{ value };
  }
};
template<typename T>
class generator_promise
{
  ...

  // Disable any use of co_await within this type of coroutine.
  template<typename U>
  std::suspend_never await_transform(U&&) = delete;

};
template<typename T, typename Executor>
class executor_task_promise
{
  Executor executor;

public:

  template<typename Awaitable>
  auto await_transform(Awaitable&& awaitable)
  {
	using cppcoro::resume_on;
	return resume_on(this->executor, std::forward<Awaitable>(awaitable));
  }
};

template<typename T>
class generator_promise
{
  T* valuePtr;
public:
  ...

  std::suspend_always yield_value(T& value) noexcept
  {
	// Stash the address of the yielded value and then return an awaitable
	// that will cause the coroutine to suspend at the co_yield expression.
	// Execution will then return from the call to coroutine_handle<>::resume()
	// inside either generator<T>::begin() or generator<T>::iterator::operator++().
	valuePtr = std::addressof(value);
	return {};
  }
};
		*/
		// source: https://github.com/lewissbaker/cppcoro/blob/master/include/cppcoro/task.hpp

		// Manual reset event
		// Is able to set and reset events
		// Will be able to perform this asynchronously

		/// \brief
		/// Exception thrown when you attempt to retrieve the result of
		/// a task that has been detached from its promise/coroutine.
		class broken_promise : public std::logic_error
		{
		public:
			broken_promise()
				: std::logic_error("broken promise")
			{
			}
		};

		template <typename T>
		class task;

		namespace detail
		{
			class task_promise_base
			{
				friend struct final_awaitable;

				struct final_awaitable
				{
					bool await_ready() const noexcept { return false; }

					template <typename PROMISE>
					std::coroutine_handle<> await_suspend(
						std::coroutine_handle<PROMISE> coro) noexcept
					{
						return coro.promise().m_continuation;
					}

					void await_resume() noexcept {}
				};

			public:
				task_promise_base() noexcept
				{
				}

				auto initial_suspend() noexcept
				{
					return std::suspend_always{};
				}

				auto final_suspend() noexcept
				{
					return final_awaitable{};
				}

				void set_continuation(std::coroutine_handle<> continuation) noexcept
				{
					m_continuation = continuation;
				}

			private:
				std::coroutine_handle<> m_continuation;
			};

			template <typename T>
			class task_promise final : public task_promise_base
			{
			public:
				task_promise() noexcept {}

				~task_promise()
				{
					switch (m_resultType)
					{
					case result_type::value:
						m_value.~T();
						break;
					case result_type::exception:
						m_exception.~exception_ptr();
						break;
					default:
						break;
					}
				}

				task<T> get_return_object() noexcept;

				void unhandled_exception() noexcept
				{
					::new (static_cast<void *>(std::addressof(m_exception))) std::exception_ptr(
						std::current_exception());
					m_resultType = result_type::exception;
				}

				template <
					typename VALUE,
					typename = std::enable_if_t<std::is_convertible_v<VALUE &&, T>>>
				void return_value(VALUE &&value) noexcept(std::is_nothrow_constructible_v<T, VALUE &&>)
				{
					::new (static_cast<void *>(std::addressof(m_value))) T(std::forward<VALUE>(value));
					m_resultType = result_type::value;
				}

				T &result() &
				{
					if (m_resultType == result_type::exception)
					{
						std::rethrow_exception(m_exception);
					}

					assert(m_resultType == result_type::value);

					return m_value;
				}

				// HACK: Need to have co_await of task<int> return prvalue rather than
				// rvalue-reference to work around an issue with MSVC where returning
				// rvalue reference of a fundamental type from await_resume() will
				// cause the value to be copied to a temporary. This breaks the
				// sync_wait() implementation.
				// See https://github.com/lewissbaker/cppcoro/issues/40#issuecomment-326864107
				using rvalue_type = std::conditional_t<
					std::is_arithmetic_v<T> || std::is_pointer_v<T>,
					T,
					T &&>;

				rvalue_type result() &&
				{
					if (m_resultType == result_type::exception)
					{
						std::rethrow_exception(m_exception);
					}

					assert(m_resultType == result_type::value);

					return std::move(m_value);
				}

			private:
				enum class result_type
				{
					empty,
					value,
					exception
				};

				result_type m_resultType = result_type::empty;

				union
				{
					T m_value;
					std::exception_ptr m_exception;
				};
			};

			template <>
			class task_promise<void> : public task_promise_base
			{
			public:
				task_promise() noexcept = default;

				task<void> get_return_object() noexcept;

				void return_void() noexcept
				{
				}

				void unhandled_exception() noexcept
				{
					m_exception = std::current_exception();
				}

				void result()
				{
					if (m_exception)
					{
						std::rethrow_exception(m_exception);
					}
				}

			private:
				std::exception_ptr m_exception;
			};

			template <typename T>
			class task_promise<T &> : public task_promise_base
			{
			public:
				task_promise() noexcept = default;

				task<T &> get_return_object() noexcept;

				void unhandled_exception() noexcept
				{
					m_exception = std::current_exception();
				}

				void return_value(T &value) noexcept
				{
					m_value = std::addressof(value);
				}

				T &result()
				{
					if (m_exception)
					{
						std::rethrow_exception(m_exception);
					}

					return *m_value;
				}

			private:
				T *m_value = nullptr;
				std::exception_ptr m_exception;
			};
		}

		/// \brief
		/// A task represents an operation that produces a result both lazily
		/// and asynchronously.
		///
		/// When you call a coroutine that returns a task, the coroutine
		/// simply captures any passed parameters and returns exeuction to the
		/// caller. Execution of the coroutine body does not start until the
		/// coroutine is first co_await'ed.
		template <typename T = void>
		class [[nodiscard]] task
		{
		public:
			using promise_type = detail::task_promise<T>;

			using value_type = T;

		private:
			struct awaitable_base
			{
				std::coroutine_handle<promise_type> m_coroutine;

				awaitable_base(std::coroutine_handle<promise_type> coroutine) noexcept
					: m_coroutine(coroutine)
				{
				}

				bool await_ready() const noexcept
				{
					return !m_coroutine || m_coroutine.done();
				}

				std::coroutine_handle<> await_suspend(
					std::coroutine_handle<> awaitingCoroutine) noexcept
				{
					m_coroutine.promise().set_continuation(awaitingCoroutine);
					return m_coroutine;
				}
			};

		public:
			task() noexcept
				: m_coroutine(nullptr)
			{
			}

			explicit task(std::coroutine_handle<promise_type> coroutine)
				: m_coroutine(coroutine)
			{
			}

			task(task &&t) noexcept
				: m_coroutine(t.m_coroutine)
			{
				t.m_coroutine = nullptr;
			}

			/// Disable copy construction/assignment.
			task(const task &) = delete;
			task &operator=(const task &) = delete;

			/// Frees resources used by this task.
			~task()
			{
				if (m_coroutine)
				{
					m_coroutine.destroy();
				}
			}

			task &operator=(task &&other) noexcept
			{
				if (std::addressof(other) != this)
				{
					if (m_coroutine)
					{
						m_coroutine.destroy();
					}

					m_coroutine = other.m_coroutine;
					other.m_coroutine = nullptr;
				}

				return *this;
			}

			/// \brief
			/// Query if the task result is complete.
			///
			/// Awaiting a task that is ready is guaranteed not to block/suspend.
			bool is_ready() const noexcept
			{
				return !m_coroutine || m_coroutine.done();
			}

			auto operator co_await() const & noexcept
			{
				struct awaitable : awaitable_base
				{
					using awaitable_base::awaitable_base;

					decltype(auto) await_resume()
					{
						if (!this->m_coroutine)
						{
							throw broken_promise{};
						}

						return this->m_coroutine.promise().result();
					}
				};

				return awaitable{m_coroutine};
			}

			auto operator co_await() const && noexcept
			{
				struct awaitable : awaitable_base
				{
					using awaitable_base::awaitable_base;

					decltype(auto) await_resume()
					{
						if (!this->m_coroutine)
						{
							throw broken_promise{};
						}

						return std::move(this->m_coroutine.promise()).result();
					}
				};

				return awaitable{m_coroutine};
			}

			/// \brief
			/// Returns an awaitable that will await completion of the task without
			/// attempting to retrieve the result.
			auto when_ready() const noexcept
			{
				struct awaitable : awaitable_base
				{
					using awaitable_base::awaitable_base;

					void await_resume() const noexcept {}
				};

				return awaitable{m_coroutine};
			}

			template <typename U, typename V = T>
				requires(!std::is_void_v<V>)
			task<U> then(std::function<U(V)> func)
			{
				auto value = co_await *this;
				co_return func(value);
			}

			template <typename V = T>
				requires(std::is_void_v<V>)
			task<void> then(std::function<void()> func)
			{
				co_await *this;
				func();
			}

			template <typename... Args>
			static task create(std::function<T(Args...)> func, Args... args)
			{
				co_return func(args...);
			}

			T run_sync()
			{
				return sync_wait(*this);
			}

		private:
			std::coroutine_handle<promise_type> m_coroutine;
		};

		namespace detail
		{
			template <typename T>
			task<T> task_promise<T>::get_return_object() noexcept
			{
				return task<T>{std::coroutine_handle<task_promise>::from_promise(*this)};
			}

			inline task<void> task_promise<void>::get_return_object() noexcept
			{
				return task<void>{std::coroutine_handle<task_promise>::from_promise(*this)};
			}

			template <typename T>
			task<T &> task_promise<T &>::get_return_object() noexcept
			{
				return task<T &>{std::coroutine_handle<task_promise>::from_promise(*this)};
			}
		}

		template <awaitable AWAITABLE>
		auto make_task(AWAITABLE awaitable)
			-> task<typename awaitable_traits<AWAITABLE>::awaiter_return_type>
		{
			co_return co_await static_cast<AWAITABLE &&>(awaitable);
		}
	}

}