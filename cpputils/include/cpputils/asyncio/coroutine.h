#include <cpputils/cpputils_api.h>
#include <cpputils/core.h>
#include <coroutine>
#include <optional>

namespace cpputils
{
	namespace asyncio
	{
		// Coroutine base class
		class CPPUTILS_API coroutine_base
		{
		public:
			coroutine_base() {}
			virtual ~coroutine_base() {}

			// Resumes the coroutine
			virtual void resume() = 0;
			// Checks if coroutine is ready
			virtual bool is_ready() = 0;
			// Checks if coroutine is done
			virtual bool is_done() = 0;
		};

		// Coroutine promise type class
		class coroutine_promise_type_base;

		template <typename T>
		class coroutine_promise_type;

		// Coroutine class
		template <typename T>
		class coroutine : public coroutine_base
		{
		private:
			// Coroutine handle
			std::coroutine_handle<coroutine_promise_type<T>> m_handle;

		public:
			// Set promise type
			using promise_type = coroutine_promise_type<T>;

			// Constructor and destructor
			coroutine(std::coroutine_handle<coroutine_promise_type<T>> handle) : m_handle(handle) {}
			// Destructor
			virtual ~coroutine()
			{
				// Destroy the handle
				if (m_handle)
				{
					m_handle.destroy();
				}
			}

			// Resumes the coroutine
			virtual void resume() override
			{
				m_handle.promise().set_ready(false);
				return m_handle.resume();
			}

			// Checks if coroutine is ready
			virtual bool is_ready() override
			{
				return m_handle.promise().is_ready();
			}

			// Checks if coroutine is done
			virtual bool is_done() override
			{
				return m_handle.promise().is_done();
			}

			// Checks if coroutine has raised exception
			virtual bool has_exception() const
			{
				return get_exception() != nullptr;
			}

			// Gets the exception
			virtual std::exception_ptr get_exception() const
			{
				return m_handle.promise().get_exception();
			}

			// Gets the return value
			auto get_return_value()
			{
				return m_handle.promise().get_return_value();
			}
		};

		// Coroutine promise type class
		class coroutine_promise_type_base
		{
		private:
			// Flags
			bool m_done;
			bool m_ready;
			std::exception_ptr ptr;

		public:
			// Constructor and destructor
			coroutine_promise_type_base() : m_done(false), m_ready(true) {}
			~coroutine_promise_type_base() {}

			// Return suspend never for now
			auto initial_suspend()
			{
				return std::suspend_always{};
			}

			// Return suspend always and set m_done to true
			auto final_suspend() noexcept
			{
				m_done = true;
				return std::suspend_always{};
			}

			// Unhandled exception
			void unhandled_exception()
			{
				ptr = std::current_exception();
			}

			void set_ready(bool ready = true)
			{
				m_ready = ready;
			}

			bool is_ready() const
			{
				return m_ready;
			}

			bool is_done() const
			{
				return m_done;
			}

			std::exception_ptr get_exception() const
			{
				return ptr;
			}
		};

		// Coroutine promise type class
		template <>
		class coroutine_promise_type<void> : public coroutine_promise_type_base
		{
		public:
			// Constructor and destructor
			coroutine_promise_type() {}
			~coroutine_promise_type() {}

			// Get return object
			auto get_return_object() { return coroutine<void>{std::coroutine_handle<coroutine_promise_type<void>>::from_promise(*this)}; }

			// Return void
			void return_void() {}

			// Get return void
			void get_return_void() {}
		};

		// Coroutine promise type class
		template <class T>
		class coroutine_promise_type : public coroutine_promise_type_base
		{
		private:
			// Return value
			std::optional<T> m_value;

		public:
			// Constructor and destructor
			coroutine_promise_type() {}
			~coroutine_promise_type() {}

			// Return value
			void return_value(const T &value)
			{
				m_value = value;
			}

			// Get return value
			std::optional<T> get_return_value()
			{
				return m_value;
			}

			// Get return object
			auto get_return_object() { return coroutine<T>{std::coroutine_handle<coroutine_promise_type<T>>::from_promise(*this)}; }
		};

		class Dispatcher;

		namespace internal
		{

			CPPUTILS_API ref<Dispatcher> get_worker_dispatcher();

			// Main dispatcher
			CPPUTILS_API ref<Dispatcher> get_main_dispatcher();

			// IO Dispatcher
			CPPUTILS_API ref<Dispatcher> get_io_dispatcher();

			// Current Dispatcher
			CPPUTILS_API ref<Dispatcher> get_current_dispatcher();
		}

		class Dispatcher
		{
		public:
			// Queues job: if function returns false, will try again til complete
			virtual void enqueue(std::function<bool()> func) = 0;

			// Run coroutine
			void run_coroutine(std::coroutine_handle<coroutine_promise_type<void>> handle, std::function<bool()> func)
			{
				enqueue(
					[handle, func]()
					{
						bool result = func();
						if (result)
							handle.promise().set_ready();
						return result;
					});
			}

			// Runs task async: will always complete
			template <typename... Args>
			void run_async(std::function<void(Args...)> func, Args... args)
			{
				enqueue(
					[func, args...]()
					{
						func(args...);
						return true; // Always return true
					});
			}

			// Worker dispatcher
			static ref<Dispatcher> get_worker_dispatcher()
			{
				return internal::get_worker_dispatcher();
			}

			// Main dispatcher
			static ref<Dispatcher> get_main_dispatcher()
			{
				return internal::get_main_dispatcher();
			}

			// IO Dispatcher
			static ref<Dispatcher> get_io_dispatcher()
			{
				return internal::get_io_dispatcher();
			}

			// Current Dispatcher
			static ref<Dispatcher> get_current_dispatcher()
			{
				return internal::get_current_dispatcher();
			}
		};

		// Awaitable class
		auto
		operator co_await(cpputils::chrono::seconds seconds)
		{
			struct awaitable
			{
				// Seconds
				cpputils::chrono::seconds m_seconds;

				// Constructor
				awaitable(cpputils::chrono::seconds seconds) : m_seconds(seconds) {}

				// Await ready
				bool await_ready() const
				{
					return m_seconds.count() <= 0;
				}

				bool coroutine_method(cpputils::chrono::time_point m_start) const
				{
					// Check if current time is greater than seconds
					auto m_end = cpputils::chrono::now();
					// Check if seconds have passed
					if (m_end - m_start >= m_seconds)
						return true;
					else
						return false;
				}

				// Await suspend
				void await_suspend(std::coroutine_handle<> handle) const
				{
					// Start time
					auto m_start = cpputils::chrono::now();
					// Get handle
					auto m_handle = std::coroutine_handle<coroutine_promise_type<void>>::from_address(handle.address());
					// Run coroutine
					Dispatcher::get_worker_dispatcher()
						->run_coroutine(m_handle, std::bind(&awaitable::coroutine_method, this, m_start));
				}

				void await_resume() const {}
			};

			return awaitable{seconds};
		}

		void run_sync(coroutine<void> coroutine)
		{
			do
			{
				if (coroutine.has_exception())
				{
					std::rethrow_exception(coroutine.get_exception());
				}

				if (coroutine.is_ready())
				{
					coroutine.resume();
				}
			} while (!coroutine.is_done());
		}
	}

}