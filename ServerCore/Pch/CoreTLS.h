#pragma once

#include <stack>
#include <string>

// 현재 소유하고 있는 LOCK
extern thread_local std::stack<uint64> LHoldLock;
// 스레드 이름
extern thread_local std::string LThreadName;
// StealJob시 마지막으로 훔친 JobGroupType 인덱스 번호
extern thread_local uint16 LStealJobIndex;