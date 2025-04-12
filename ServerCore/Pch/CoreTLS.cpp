#include "pch.h"
#include "coreTLS.h"

thread_local std::stack<uintptr_t> LThreadLock;