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
			ObjectPool<Session>::Release(pair.second);
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

// 세션 해제
// 등록되지 않은 세션용
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

// 세션 삭제
void SessionManager::Remove(SessionID sessionID)
{
	LOCK_GUARD;

	auto findIt = _sessions.find(sessionID);
	if (findIt != _sessions.end())
	{
		Session* removeSession = findIt->second;
		_sessions.erase(findIt);
		ObjectPool<Session>::Release(removeSession);
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
}
