#include <cpputils/asyncio/coroutine.h>
#include <future>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace cpputils::asyncio;
using namespace cpputils;

class IODispatcher : public Dispatcher
{
public:
	// Queues job: if function returns false, will try again til complete
	void enqueue(std::function<bool()> func)
	{
		// TODO: implement this
	}
};

class MainDispatcher : public Dispatcher
{
private:
	cpputils::queue<std::function<bool()>> m_queue;

public:
	MainDispatcher()
	{
	}
	// Queues job: if function returns false, will try again til complete
	void enqueue(std::function<bool()> func)
	{
		m_queue.push(func);
	}

	// Event loop
	void run()
	{
		while (!m_queue.empty())
		{
			auto func = m_queue.front();
			m_queue.pop();
			if (!func())
			{
				m_queue.push(func);
			}
		}
	}
};

class WorkerDispatcher : public Dispatcher
{
private:
	cpputils::queue<std::function<bool()>> m_queue;
	array_list<std::thread> m_threads;
	std::mutex m_mutex;
	std::condition_variable m_condition;
	bool m_stop = false;

public:
	WorkerDispatcher(size_t num_threads)
	{
		for (size_t i = 0; i < num_threads; i++)
		{
			m_threads.emplace_back([this]()
								   { event_loop(); });
		}
	}

	~WorkerDispatcher()
	{
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			m_stop = true;
		}

		m_condition.notify_all();

		for (auto &thread : m_threads)
		{
			thread.join();
		}
	}

	// Queues job: if function returns false, will try again til complete
	void enqueue(std::function<bool()> func)
	{
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			m_queue.push(func);
		}
		m_condition.notify_one();
	}

	void event_loop()
	{
		while (true)
		{
			std::function<bool()> task;
			// The reason for putting the below code
			// here is to unlock the queue before
			// executing the task so that other
			// threads can perform enqueue tasks
			{
				// Locking the queue so that data
				// can be shared safely
				std::unique_lock<std::mutex> lock(
					m_mutex);

				// Waiting until there is a task to
				// execute or the pool is stopped
				m_condition.wait(lock, [this]
								 { return !m_queue.empty() || m_stop; });

				// exit the thread in case the pool
				// is stopped and there are no tasks
				if (m_stop && m_queue.empty())
				{
					return;
				}

				// Get the next task from the queue
				task = std::move(m_queue.front());
				m_queue.pop();
			}

			if (!task())
			{
				enqueue(task);
			}
		}
	}
};

ref<Dispatcher> cpputils::asyncio::internal::get_current_dispatcher()
{
	return nullptr;
}

static ref<Dispatcher> main_dispatcher = make_ref<MainDispatcher>();
static ref<Dispatcher> worker_dispatcher = make_ref<WorkerDispatcher>(std::thread::hardware_concurrency());
static ref<Dispatcher> io_dispatcher = make_ref<IODispatcher>();

ref<Dispatcher> cpputils::asyncio::internal::get_main_dispatcher()
{
	return main_dispatcher;
}

ref<Dispatcher> cpputils::asyncio::internal::get_worker_dispatcher()
{
	return worker_dispatcher;
}

ref<Dispatcher> cpputils::asyncio::internal::get_io_dispatcher()
{
	return io_dispatcher;
}