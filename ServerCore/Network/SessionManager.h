#pragma once
#include "Session.h"

/*-------------------------------------------------------
				SessionManager

- Session 생명주기 관리
--------------------------------------------------------*/

class SessionManager : public Singleton<SessionManager>
{
public:
	void Init() override;
	void Shutdown() override;
	// 세션 생성
	SessionRef Create();
	// 세션 추가
	void Add(SessionRef session);
	// 세션 찾기
	SessionRef Find(SessionID sessionID);
	// 타임아웃 세션 닫기
	void Tick();

private:
	USE_LOCK;
	// 세션 목록
	HashMap<SessionID, SessionRef> _sessions;
	TimePoint _lastCleanUpTime;
};
