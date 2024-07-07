#include <cpputils/cpputils.h>
#include <cpputils/asyncio/coroutine.h>
#include <chrono>

using namespace std::chrono_literals;
using namespace cpputils::asyncio;

struct scheduler
{
	using task_func = std::function<bool()>;

private:
	std::queue<task_func> task_queue;
	std::thread running_thread;
	bool m_stop = false;
	std::mutex m_mutex;
	std::condition_variable cv;

public:
	scheduler()
	{
		running_thread = std::thread(&scheduler::event_loop, this);
	}

	~scheduler()
	{
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_stop = true;
		}
		cv.notify_one();
		running_thread.join();
	}

	void event_loop()
	{
		while (!m_stop)
		{
			task_func job;
			{
				std::unique_lock<std::mutex> lock(m_mutex);
				cv.wait(lock, [&]
						{ return !task_queue.empty() || m_stop; });
				if (task_queue.empty() && m_stop)
				{
					return;
				}
				job = task_queue.front();
				task_queue.pop();
			}
			try
			{
				if (!job())
					queue(job);
			}
			catch (std::runtime_error err)
			{
				std::cerr << err.what() << std::endl;
			}
		}
	}

	void queue(task_func func)
	{
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			task_queue.push(func);
		}
		cv.notify_one();
	}
};

static scheduler sc;

task<void> sleep(std::chrono::seconds delay)
{
	async_manual_reset_event event;
	auto start = std::chrono::steady_clock::now();

	sc.queue([&, start, d = delay]
			 {
			if (decltype(start)::clock::now() - start > d) {
				event.set();
				return true;
			} else {
				return false;
			} });

	co_await event;
}

// Not thread safe yet
// logging configuration wasnt applied completely
// Lets modify to a more event based apporach
auto operator co_await(std::chrono::seconds delay)
{
	struct sleep_awaitable
	{
		sleep_awaitable(std::chrono::seconds d) : delay(d) {}

		bool await_ready() const noexcept { return false; }

		void await_suspend(std::coroutine_handle<> h) const
		{
			sleep(delay).run_sync();
			h.resume();
		}

		void await_resume() const noexcept {}

		std::chrono::seconds delay;
	};

	return sleep_awaitable{delay};
}

task<void> test_local_logger()
{
	LOG_DEBUG("GETTING LOCAL LOGGER IN 2 seconds");
	co_await 2s;

	// Getting new logger
	auto logger = cpputils::Debug::get_logger("Cool Name");

	LOG_DEBUG("TESTING LOCAL LOGGER IN 2 seconds");
	co_await 2s;
	//  Testing config setting
	cpputils::Debug::get_global_logger()->set_config(cpputils::log_level::INFO);
	//  Calling hello world from logger
	LOGGER_LOG_INFO(logger, "Hello World From Logger: {}", logger->name());
	// This should not print
	LOGGER_LOG_DEBUG(logger, "Logging Injection {} \\{{}\\} {}", 5, 12.05, "Meee");
}

// Coroutine function
task<void> coroutine_func()
{
	// Testing hello world
	LOG_DEBUG("Hello, World!");
	co_await 1s;
	LOG_DEBUG("Hello, World! After 1s");
	// Testing math
	LOG_DEBUG("add(1, 2) = {}", add(1, 2));
	LOG_DEBUG("sub(1, 2) = {}", sub(1, 2));
	LOG_DEBUG("mul(1, 2) = {}", mul(1, 2));
	LOG_DEBUG("divi(1, 2) = {}", divi(1, 2));

	LOG_DEBUG("SETTING CONFIG IN 2 seconds");
	co_await 2s;

	co_await test_local_logger();
}

int main(int argc, char **argv)
{
	auto task = coroutine_func();
	task.run_sync();
	return 0;
}