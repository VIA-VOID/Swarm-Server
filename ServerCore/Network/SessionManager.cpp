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

// 세션 하트비트 & 만료 세션 닫기
void SessionManager::Heartbeat(CoreService* service)
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

	// 활성화 세션
	Vector<SessionRef> liveSessions;
	// 타임아웃 세션
	Vector<SessionRef> timeoutSessions;
	
	for (auto& session : sessions)
	{
		if (session == nullptr)
		{
			continue;
		}
		if (session->IsClosed() || session->IsTimeout())
		{
			timeoutSessions.push_back(session);
			continue;
		}
		liveSessions.push_back(session);
	}

	if (timeoutSessions.empty() == false)
	{
		// 세션 닫기
		for (auto& session : timeoutSessions)
		{
			if (session->IsTimeout())
			{
				LOG_WARNING("세션 타임아웃: " + std::to_string(session->GetSessionID().GetID()));
			}
			session->Close();
		}
		// 세션 제거
		{
			LOCK_GUARD;

			for (auto& session : timeoutSessions)
			{
				_sessions.erase(session->GetSessionID());
			}
		}
	}

	// 하트비트 패킷 전송
	if (service != nullptr)
	{
		for (auto& session : liveSessions)
		{
			// 클라이언트가 일정 수만큼 응답이 없을 경우 세션 닫음
			if (session->GetPingCount() - session->GetPongCount() >= PING_PONG_DIFF)
			{
				session->Close();
			}
			else
			{
				service->OnHeartbeat(session);
			}
		}
	}
}
