#include "pch.h"
#include "coreTLS.h"

// 현재 소유하고 있는 LOCK
thread_local std::stack<uint64> LHoldLock;
// 스레드 이름
thread_local std::string LThreadName;
// 스레드 그룹 ID
thread_local JobGroupId LJobGroupId;
// 현재일자
thread_local std::string LToday;