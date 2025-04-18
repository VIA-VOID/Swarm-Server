#include "pch.h"
#include "coreTLS.h"

// 현재 소유하고 있는 LOCK
thread_local std::stack<uint64> LHoldLock;
