#pragma once

/*----------------------------
	MemoryLockStack

- std::stack wrapping
- MemoryPool에서 사용
----------------------------*/
template <typename T>
class MemoryLockStack
{
public:
	void Push(const T& item)
	{
		LOCK_GUARD;
		_stack.push(item);
	}

	T Pop()
	{
		LOCK_GUARD;
		if (_stack.empty())
		{
			return nullptr;
		}
		T item = _stack.top();
		_stack.pop();
		return item;
	}

	T& Top()
	{
		LOCK_GUARD;
		return _stack.top();
	}

	bool Empty()
	{
		LOCK_GUARD;
		return _stack.empty();
	}

private:
	USE_LOCK;
	std::stack<T> _stack;
};
