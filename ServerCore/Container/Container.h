#pragma once
#include "Memory/STLAllocator.h"

#include <array>
#include <vector>
#include <list>
#include <queue>
#include <stack>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>

template <typename Type, uint32 Size>
using Array = std::array<Type, Size>;

template <typename Type>
using Vector = std::vector<Type, STLAllocator<Type>>;

template <typename Type>
using List = std::list<Type, STLAllocator<Type>>;

template <typename Key, typename Type, typename Pred = std::less<Key>>
using Map = std::map<Key, Type, Pred, STLAllocator<std::pair<const Key, Type>>>;

template <typename Key, typename Pred = std::less<Key>>
using Set = std::set<Key, Pred, STLAllocator<Key>>;

template <typename Type>
using Deque = std::deque<Type, STLAllocator<Type>>;

template <typename Type, typename Container = Deque<Type>>
using Queue = std::queue<Type, Container>;

template <typename Type, typename Container = Deque<Type>>
using Stack = std::stack<Type, Container>;

template <typename Type, typename Container = Vector<Type>, typename Pred = std::less<typename Container::value_type>>
using PriorityQueue = std::priority_queue<Type, Container, Pred>;

template <typename Key, typename Type, typename Hasher = std::hash<Key>, typename KeyEq = std::equal_to<Key>>
using HashMap = std::unordered_map<Key, Type, Hasher, KeyEq, STLAllocator<std::pair<const Key, Type>>>;

template <typename Key, typename Hasher = std::hash<Key>, typename KeyEq = std::equal_to<Key>>
using HashSet = std::unordered_set<Key, Hasher, KeyEq, STLAllocator<Key>>;
