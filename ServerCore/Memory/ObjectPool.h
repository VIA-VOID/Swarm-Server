#pragma once

/*-------------------------------------------------------
				ObjectPool

- MemoryPool에서 할당
--------------------------------------------------------*/

template <typename T>
class ObjectPool : public Singleton<ObjectPool>
{
public:
	// 메모리 대여
	template <typename ...Args>
	void* Allocate(Args&&... args);
	// 메모리 반납
	void Release(T* object);
	// 스마트포인터로 생성/소멸
	template <typename ...Args>
	std::shared_ptr<T> MakeShared(Args&&... args);
};

// 메모리 대여
template <typename T>
template <typename ...Args>
inline void* ObjectPool<T>::Allocate(Args&&... args)
{
	T* memory = MEMORY_MANAGER.Allocate(sizeof(T));
	// placement new
	new(memory)T(std::forward<Args>(args)...);
	return memory;
}

// 메모리 반납
template<typename T>
inline void ObjectPool<T>::Release(T* object)
{
	object->~T();
	MEMORY_MANAGER.Release(object);
}

// 스마트포인터로 생성/소멸
template<typename T>
template<typename ...Args>
inline std::shared_ptr<T> ObjectPool<T>::MakeShared(Args&&... args)
{
	return { Allocate(std::forward<Args>(args)...), Release };
}
