#ifndef SEMAPHORE_H
#define SEMAPHORE_H
#include <mutex>
#include <condition_variable>

class Semaphore
{
public:
	Semaphore()
	{
		count_ = 0;
	}
	~Semaphore(){}

	void Post(unsigned int n)
	{
		std::unique_lock<std::mutex> lock(mtx_);
		if (n == 1)
		{
			condiction_.notify_one();
		}
		else
		{
			condiction_.notify_all();
		}
	}

	void Wait()
	{
		std::unique_lock<std::mutex> lock(mtx_);
		if (count_ == 0)
		{
			condiction_.wait(lock);
		}
		count_--;
	}
private:
	int count_;
	std::mutex mtx_;
	std::condition_variable_any condiction_;
};

#endif