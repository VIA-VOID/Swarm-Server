#pragma once

/*-------------------------------------------------------

- Protobuf 전역 연산자 오버로딩
- 필요에 따라 추가

--------------------------------------------------------*/

// StatInfo 전역 연산자 오버로딩
Protocol::StatInfo& operator+=(Protocol::StatInfo& statInfo, const Protocol::StatInfo& otherInfo);