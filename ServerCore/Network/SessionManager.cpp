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
		_sessions.clear();
		_deleteSessions.clear();
	}
}

// 세션 생성 및 추가
SessionRef SessionManager::Create()
{
	return ObjectPool<Session>::MakeShared();
}

// 세션 추가
void SessionManager::Add(SessionRef session)
{
	if (session == nullptr)
	{
		return;
	}
	{
		LOCK_GUARD;
		SessionID sessionID = session->GetSessionID();
		_sessions.insert({ sessionID, session });
	}
}

// 세션이 종료된 후 호출
void SessionManager::OnSessionClosed(SessionRef session)
{
	if (session == nullptr)
	{
		return;
	}
	{
		LOCK_GUARD;

		SessionID sessionID = session->GetSessionID();
		auto it = _sessions.find(sessionID);
		if (it != _sessions.end())
		{
			// 세션 목록에서 제거
			_sessions.erase(it);
		}
		// 삭제 대기 추가
		_deleteSessions.push_back(session);
	}
}

// 세션 찾기
SessionRef SessionManager::Find(SessionID sessionID)
{
	LOCK_GUARD;

	auto it = _sessions.find(sessionID);
	if (it != _sessions.end())
	{
		return it->second;
	}
	return nullptr;
}

// 타임아웃 세션 닫기
void SessionManager::Tick()
{
	// 타임아웃된 세션 찾기
	Vector<SessionRef> timeoutSessions;
	{
		LOCK_GUARD;
		timeoutSessions.reserve(_sessions.size() / 10);

		for (auto& pair : _sessions)
		{
			auto& session = pair.second;
			if (session != nullptr && session->IsTimeout())
			{
				timeoutSessions.push_back(session);
			}
		}
	}

	// 타임아웃 세션 닫기
	for (auto& session : timeoutSessions)
	{
		LOG_WARNING("세션 타임아웃: " + std::to_string(session->GetSessionID().GetID()));
		session->Close();
	}

	// 주기적으로 삭제 대기 중인 세션 정리
	if (NOW - _lastCleanUpTime >= CLEANUP_INTERVAL)
	{
		CleanUpSessions();
		_lastCleanUpTime = NOW;
	}
}

// 삭제 대기 중인 세션 정리
void SessionManager::CleanUpSessions()
{
	LOCK_GUARD;
	for (auto it = _deleteSessions.begin(); it != _deleteSessions.end();)
	{
		SessionRef session = *it;
		// 참조 카운트가 없다면(_deleteSessions 에만 존재)
		if (session.use_count() <= 1)
		{
			it = _deleteSessions.erase(it);
		}
		else
		{
			++it;
		}
	}
}
