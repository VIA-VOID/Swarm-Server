#include "pch.h"
#include "SessionManager.h"
#include "Utils/Timer.h"

/*----------------------------
		SessionManager
----------------------------*/

void SessionManager::Init()
{
	_lastCleanUpTime = NOW;
}

void SessionManager::Shutdown()
{
	// 모든 세션 정리
	{
		LOCK_GUARD;

		// 삭제 대기 세션 정리
		for (auto session : _deleteSessions)
		{
			ObjectPool<Session>::Release(session);
		}
		_deleteSessions.clear();

		// 남아있는 세션 정리
		for (auto& pair : _sessions)
		{
			Session* session = pair.second;
			session->SetState(SessionState::Closed);
			ObjectPool<Session>::Release(session);
		}
		_sessions.clear();
	}
}

// 세션 생성 및 추가
Session* SessionManager::Create()
{
	return ObjectPool<Session>::Allocate();
}

// 세션 추가
void SessionManager::Add(Session* session)
{
	if (session == nullptr)
	{
		return;
	}
	LOCK_GUARD;
	SessionID sessionID = session->GetSessionID();
	_sessions.insert({ sessionID, session });
}

// _sessions에 등록되지 않은 세션용
// 세션 해제 + 자원 해제
void SessionManager::Release(Session* session)
{
	if (session == nullptr)
	{
		return;
	}
	// 세션이 이미 매니저에 등록되어 있는지 확인 (중복 해제 방지)
	// 이미 등록된 세션은 Remove를 통해 정리
	LOCK_GUARD;
	auto it = _sessions.find(session->GetSessionID());
	if (it != _sessions.end() && it->second == session)
	{
		LOG_WARNING(L"이미 등록된 세션입니다. Remove를 통해 제거");
		return;
	}
	ObjectPool<Session>::Release(session);
}

// 세션이 종료된 후 호출
void SessionManager::OnSessionClosed(Session* session)
{
	if (session == nullptr)
	{
		return;
	}

	LOCK_GUARD;

	auto it = _sessions.find(session->GetSessionID());
	if (it != _sessions.end() && it->second == session)
	{
		// 세션 목록에서 제거
		_sessions.erase(it);

		// 세션 상태가 CloseRequest 상태가 아니면 설정
		if (session->GetState() != SessionState::CloseRequest)
		{
			session->SetState(SessionState::CloseRequest);
		}

		// 삭제 대기 추가
		_deleteSessions.push_back(session);
	}
}

// 활성상태인 세션 지연삭제
void SessionManager::Remove(SessionID sessionID)
{
	LOCK_GUARD;

	auto it = _sessions.find(sessionID);
	if (it != _sessions.end())
	{
		Session* removeSession = it->second;
		// 지연 삭제
		if (removeSession->IsActive())
		{
			removeSession->Close();
		}
	}
}

// 세션 찾기
Session* SessionManager::Find(SessionID sessionID)
{
	LOCK_GUARD;

	auto it = _sessions.find(sessionID);
	if (it != _sessions.end())
	{
		return it->second;
	}
	return nullptr;
}

// 타임아웃 세션 관리
void SessionManager::Tick()
{
	// 타임아웃된 세션 찾기
	Vector<Session*> timeoutSessions;
	{
		LOCK_GUARD;
		timeoutSessions.reserve(_sessions.size() / 10);

		for (auto& pair : _sessions)
		{
			auto& session = pair.second;
			if (session->IsTimeout())
			{
				timeoutSessions.push_back(session);
			}
		}
	}

	// 타임아웃 세션 닫기
	for (auto& session : timeoutSessions)
	{
		LOG_WARNING(L"세션 타임아웃: " + std::to_wstring(session->GetSessionID().GetID()));
		session->Close();
	}

	// 주기적으로 삭제 대기 중인 세션 정리
	if (NOW - _lastCleanUpTime >= CLEANUP_INTERVAL)
	{
		CleanUpSession();
		_lastCleanUpTime = NOW;
	}
}

// 삭제 대기 중인 세션 정리
void SessionManager::CleanUpSession()
{
	// 실제 삭제할 세션 배열
	Vector<Session*> sessions;
	{
		LOCK_GUARD;
		for (auto it = _deleteSessions.begin(); it != _deleteSessions.end();)
		{
			Session* session = *it;
			if (session->GetState() == SessionState::CloseRequest)
			{
				// 세션 목록 삭제
				it = _deleteSessions.erase(it);
				// 삭제 예정 추가
				sessions.push_back(session);
			}
			else
			{
				++it;
			}
		}
	}

	// 실제 메모리 해제
	for (Session* session : sessions)
	{
		ObjectPool<Session>::Release(session);
	}
}
