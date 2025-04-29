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
	// 세션 생성
	Session* Create();
	// 세션 추가
	void Add(Session* session);
	// 세션 해제
	// 등록되지 않은 세션용
	void Release(Session* session);
	// 세션 삭제
	// 등록된 세션 제거
	void Remove(SessionID sessionID);
	// 세션 찾기
	Session* Find(SessionID sessionID);
	// 타임아웃 세션 관리
	void Tick();

private:
	USE_LOCK;
	HashMap<SessionID, Session*> _sessions;
};
