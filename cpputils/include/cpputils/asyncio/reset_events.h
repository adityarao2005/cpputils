#pragma once

#include <cpputils/cpputils_api.h>
#include <cpputils/core.h>
#include <coroutine>
#include <atomic>
#include <mutex>
#include <condition_variable>

namespace cpputils
{
	namespace asyncio
	{
		// Events stuff

		// Manual reset event
		class manual_reset_event
		{
		private:
			std::mutex m_mutex;
			std::condition_variable m_cv;
			std::atomic<bool> m_isSet;

		public:
			manual_reset_event(bool initiallySet = false) noexcept
				: m_isSet(initiallySet)
			{
			}

			~manual_reset_event() noexcept = default;

			manual_reset_event(const manual_reset_event &) = delete;
			manual_reset_event &operator=(const manual_reset_event &) = delete;

			void set() noexcept
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				m_isSet = true;
				m_cv.notify_all();
			}
			void reset() noexcept
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				m_isSet = false;
			}

			void wait() noexcept
			{
				std::unique_lock<std::mutex> lock(m_mutex);
				m_cv.wait(lock, [this]
						  { return m_isSet == true; });
			}
			bool wait_for(std::chrono::nanoseconds timeout) noexcept
			{
				std::unique_lock<std::mutex> lock(m_mutex);
				return m_cv.wait_for(lock, timeout, [this]
									 { return m_isSet == true; });
			}
		};

		class async_manual_reset_event
		{
		public:
			async_manual_reset_event(
				bool initiallySet = false) noexcept
				: m_state(initiallySet ? this : nullptr)
			{
			}
			~async_manual_reset_event() noexcept = default;

			async_manual_reset_event(const async_manual_reset_event &) = delete;
			async_manual_reset_event &operator=(const async_manual_reset_event &) = delete;

			struct awaiter
			{
				awaiter(const async_manual_reset_event &event) noexcept
					: m_event(event)
				{
				}
				bool await_ready() const noexcept
				{
					return m_event.is_set();
				}
				bool await_suspend(
					std::coroutine_handle<> awaitingCoroutine) noexcept
				{
					// Special m_state value that indicates the event is in the 'set' state.
					const void *const setState = &m_event;

					// Remember the handle of the awaiting coroutine.
					m_awaitingCoroutine = awaitingCoroutine;

					// Try to atomically push this awaiter onto the front of the list.
					void *oldValue = m_event.m_state.load(std::memory_order_acquire);
					do
					{
						// Resume immediately if already in 'set' state.
						if (oldValue == setState)
							return false;

						// Update linked list to point at current head.
						m_next = static_cast<awaiter *>(oldValue);

						// Finally, try to swap the old list head, inserting this awaiter
						// as the new list head.
					} while (!m_event.m_state.compare_exchange_weak(
						oldValue,
						this,
						std::memory_order_release,
						std::memory_order_acquire));

					// Successfully enqueued. Remain suspended.
					return true;
				}
				void await_resume() noexcept {}

			private:
				friend class async_manual_reset_event;
				const async_manual_reset_event &m_event;
				std::coroutine_handle<> m_awaitingCoroutine;
				awaiter *m_next;
			};

			awaiter operator co_await() const noexcept
			{
				return awaiter{*this};
			}

			void set() noexcept
			{
				// Needs to be 'release' so that subsequent 'co_await' has
				// visibility of our prior writes.
				// Needs to be 'acquire' so that we have visibility of prior
				// writes by awaiting coroutines.
				void *oldValue = m_state.exchange(this, std::memory_order_acq_rel);
				if (oldValue != this)
				{
					// Wasn't already in 'set' state.
					// Treat old value as head of a linked-list of waiters
					// which we have now acquired and need to resume.
					auto *waiters = static_cast<awaiter *>(oldValue);
					while (waiters != nullptr)
					{
						// Read m_next before resuming the coroutine as resuming
						// the coroutine will likely destroy the awaiter object.
						auto *next = waiters->m_next;
						waiters->m_awaitingCoroutine.resume();
						waiters = next;
					}
				}
			}
			void reset() noexcept
			{
				void *oldValue = this;
				m_state.compare_exchange_strong(oldValue, nullptr, std::memory_order_acquire);
			}
			bool is_set() const noexcept
			{
				return m_state.load(std::memory_order_acquire) == this;
			}

		private:
			friend struct awaiter;

			// - 'this' => set state
			// - otherwise => not set, head of linked list of awaiter*.
			mutable std::atomic<void *> m_state;
		};

	}
}