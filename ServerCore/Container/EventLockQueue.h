#pragma once

/*--------------------------------------------------------
					EventLockQueue

- std::queue wrapping(unique_lock + condition_variable)
--------------------------------------------------------*/
template <typename T>
class EventLockQueue
{
public:
	void Push(const T& item)
	{
		{
			UNIQUE_LOCK_GUARD;
			_queue.push(item);
		}
		_cv.notify_one();
	}

	T Pop()
	{
		UNIQUE_LOCK_GUARD;
		_cv.wait(ulockGuard, [&]()
			{
				return _queue.empty() == false;
			}
		);
		T item = _queue.front();
		_queue.pop();
		return item;
	}

	bool Empty()
	{
		UNIQUE_LOCK_GUARD;
		return _queue.empty();
	}

private:
	USE_LOCK;
	std::queue<T> _queue;
	ConditionVariable _cv;
};
