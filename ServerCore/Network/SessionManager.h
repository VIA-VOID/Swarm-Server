#pragma once
#include "Session.h"

/*-------------------------------------------------------
				SessionManager

- Session 생명주기 관리
- 검색 및 순회 기능
- 타임아웃 처리
--------------------------------------------------------*/

class SessionManager : public Singleton<SessionManager>
{
public:
	void Init() override;
	void Shutdown() override;
	// 세션 추가
	void Add(SessionRef session);
	// 세션 삭제
	void Remove(SessionID sessionID);
	// 세션 찾기
	SessionRef Find(SessionID sessionID);
	// 타임아웃 세션 관리
	void Tick();

private:
	USE_LOCK;
	HashMap<SessionID, SessionRef> _sessions;
};
