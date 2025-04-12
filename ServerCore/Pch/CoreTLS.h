#pragma once

#include <stack>

extern thread_local std::stack<uintptr_t> LThreadLock;