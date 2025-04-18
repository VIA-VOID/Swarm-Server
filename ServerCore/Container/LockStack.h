#pragma once

/*----------------------------
	LockStack

- std::stack wrapping
----------------------------*/
template <typename T>
class LockStack
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
