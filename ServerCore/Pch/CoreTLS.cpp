#include "pch.h"
#include "coreTLS.h"

// 현재 소유하고 있는 LOCK
thread_local std::stack<uint64> LHoldLock;
// 스레드 이름
thread_local std::string LThreadName;
// StealJob시 마지막으로 훔친 JobGroupId
thread_local JobGroupId LStealJobGroupId;