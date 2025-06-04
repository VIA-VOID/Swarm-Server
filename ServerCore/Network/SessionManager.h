#pragma once
#include "Session.h"

/*-------------------------------------------------------
				SessionManager

- Session 생명주기 관리
- 검색 및 순회 기능
- 타임아웃 처리
- 지연삭제 처리, 참조 카운트 기반
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
	// _sessions에 등록되지 않은 세션용
	// 세션 해제 + 자원 해제
	void Release(Session* session);
	// 세션이 종료된 후 호출
	void OnSessionClosed(Session* session);
	// 활성상태인 세션 지연삭제
	void Remove(SessionID sessionID);
	// 세션 찾기
	Session* Find(SessionID sessionID);
	// 타임아웃 세션 관리
	void Tick();
	// 현재 세션 수 가져오기
	uint32 GetCurrentSessionCount();

private:
	// 삭제 대기 중인 세션 정리
	void CleanUpSession();

private:
	USE_LOCK;
	// 세션 목록
	HashMap<SessionID, Session*> _sessions;
	// 삭제 대기 세션 목록
	Vector<Session*> _deleteSessions;
	TimePoint _lastCleanUpTime;

};
