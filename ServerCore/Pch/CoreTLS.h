#pragma once

#include <stack>

// 현재 소유하고 있는 LOCK
extern thread_local std::stack<uint64> LHoldLock;