#pragma once

//----------------------------------------------------------//
// 자료형 정의
using BYTE = unsigned char;
using int8 = __int8;
using int16 = __int16;
using int32 = __int32;
using int64 = __int64;
using uint8 = unsigned __int8;
using uint16 = unsigned __int16;
using uint32 = unsigned __int32;
using uint64 = unsigned __int64;

//----------------------------------------------------------//
// 조건부 타입 정의
#ifdef _DEBUG
using ConditionVariable = std::condition_variable_any;
#else
using ConditionVariable = std::condition_variable;
#endif
