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
	// 타임아웃된 세션 찾기
	Vector<SessionRef> timeoutSessions;
	{
		LOCK_GUARD;

		for (auto it = _sessions.begin(); it != _sessions.end();)
		{
			auto& session = it->second;
			
			// 이미 닫혔으면 제거
			if (session == nullptr || session->IsClosed())
			{
				it = _sessions.erase(it);
				continue;
			}

			// 타임아웃 체크
			if (session->IsTimeout())
			{
				LOG_WARNING("세션 타임아웃: " + std::to_string(session->GetSessionID().GetID()));
				timeoutSessions.push_back(session);
				it = _sessions.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	// 타임아웃된 세션 닫기
	for (auto& session : timeoutSessions)
	{
		if (session->IsClosed() == false)
		{
			session->Close();
		}
	}
}
