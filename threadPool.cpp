

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic> 
#include <deque>
#include <vector>
#include <queue>
#include <map>
#include <string>
#include <time.h>

using namespace std;
std::mutex handls_mutex;

namespace myThreadPool
{
	template<typename T>
	class thread_safe_queue
	{
	private:
		mutable std::mutex mut;
		std::queue<T> data_queue;
		std::condition_variable data_cond;

	public:
		thread_safe_queue() {}

		void push(T new_value)
		{
			std::lock_guard<std::mutex> lk(mut);
			data_queue.push(std::move(new_value));
			data_cond.notify_one();
		}

		void wait_and_pop(T& value)
		{
			std::unique_lock<std::mutex> lk(mut);
			data_cond.wait(lk, [this] {return !data_queue.empty(); });
			value = std::move(data_queue.front());
			data_queue.pop();
		}

		std::shared_ptr<T> wait_and_pop()
		{
			std::unique_lock<std::mutex> lk(mut);
			data_cond.wait(lk, [this] {return !data_queue.empty(); });
			std::shared_ptr<T> res(std::make_shared<T>(std::move(data_queue.front())));
			data_queue.pop();
			return res;
		}

		bool try_pop(T& value)
		{
			std::lock_guard<std::mutex> lk(mut);
			if (data_queue.empty())
				return false;
			value = std::move(data_queue.front());
			data_queue.pop();
			return true;
		}

		std::shared_ptr<T> try_pop()
		{
			std::lock_guard<std::mutex> lk(muk);
			if (data_queue.empty())
				return std::shared_ptr<T>();
			std::shared_ptr<T> res(std::make_shared<T>(std::move(data_queue.front())));
			data_queue.pop();
			return res;
		}

		bool empty() const
		{
			std::lock_guard<std::mutex> lk(mut);
			return data_queue.empty();
		}
	};

	class join_threads
	{
		std::vector<std::thread>& threads;
	public:
		explicit join_threads(std::vector<std::thread>& threads_) : threads(threads_) {}
		~join_threads()
		{
			for (unsigned long i = 0; i < threads.size(); ++i)
			{
				if (threads[i].joinable())
					threads[i].join();
			}
		}
	};

	class thread_pool
	{
		std::atomic_bool done;
		thread_safe_queue<std::function<void()>> work_queue;
		std::vector<std::thread> threads;
		join_threads joiner;

		void worker_thread()
		{
			while (!done)
			{
				std::function<void()> task;
				if (work_queue.try_pop(task))
					task();
				else
					std::this_thread::yield();
			}
		}
	public:
		thread_pool() :done(false), joiner(threads)
		{
			unsigned const thread_count = std::thread::hardware_concurrency();

			try
			{
				for (unsigned i = 0; i < thread_count; ++i)
				{
					threads.push_back(std::thread(&thread_pool::worker_thread, this));
				}
			}
			catch (...)
			{
				done = true;
				throw;
			}
		}
		~thread_pool()
		{
			done = true;
		}

		template<typename FunctionType>
		void submit(FunctionType f)
		{
			work_queue.push(std::function<void()>(f));
		}
	};

}
