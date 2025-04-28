#include "pch.h"
#include "SessionManager.h"
#include "Utils/Timer.h"

/*----------------------------
		SessionManager
----------------------------*/

void SessionManager::Init()
{
	LOG_SYSTEM(L"SessionManager instance initialized");
}

void SessionManager::Shutdown()
{
	// 모든 세션 정리
	{
		LOCK_GUARD;

		for (auto& pair : _sessions)
		{
			pair.second->Close();
		}
		_sessions.clear();
	}
}

// 세션 추가
void SessionManager::Add(SessionRef session)
{
	if (session == nullptr)
	{
		return;
	}
	LOCK_GUARD;
	SessionID sessionID = session->GetSessionID();
	_sessions.insert({ sessionID, session });
}

// 세션 삭제
void SessionManager::Remove(SessionID sessionID)
{
	LOCK_GUARD;

	auto it = _sessions.find(sessionID);
	if (it != _sessions.end())
	{
		_sessions.erase(it);
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

// 타임아웃 세션 관리
void SessionManager::Tick()
{
	// 타임아웃된 세션 찾기
	Vector<SessionRef> timeoutSessions;
	{
		LOCK_GUARD;
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
}
