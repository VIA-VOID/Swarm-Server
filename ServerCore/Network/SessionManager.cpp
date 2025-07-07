#include "pch.h"
#include "SessionManager.h"
#include "Utils/Timer.h"

/*----------------------------
		SessionManager
----------------------------*/

void SessionManager::Init()
{
}

void SessionManager::Shutdown()
{
	// 모든 세션 정리
	{
		LOCK_GUARD;
		_sessions.clear();
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
	Vector<SessionRef> sessions;
	{
		LOCK_GUARD;

		sessions.reserve(_sessions.size());
		
		for (const auto& pair : _sessions)
		{
			sessions.push_back(pair.second);
		}
	}

	// 타임아웃 세션
	Vector<SessionRef> timeoutSessions;
	// 닫힌 세션
	Vector<SessionRef> closedSessions;
	
	for (auto& session : sessions)
	{
		if (session == nullptr)
		{
			continue;
		}
		if (session->IsClosed())
		{
			closedSessions.push_back(session);
			continue;
		}
		if (session->IsTimeout())
		{
			timeoutSessions.push_back(session);
		}
	}

	// 닫힌 세션 제거
	if (closedSessions.empty() == false)
	{
		LOCK_GUARD;

		for (auto& session : closedSessions)
		{
			_sessions.erase(session->GetSessionID());
		}
	}

	// 타임아웃 세션 닫기
	for (auto& session : timeoutSessions)
	{
		LOG_WARNING("세션 타임아웃: " + std::to_string(session->GetSessionID().GetID()));
		session->Close();
	}
}
