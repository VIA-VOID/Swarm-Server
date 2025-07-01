#pragma once

/*-------------------------------------
	싱글톤 패턴 적용된 클래스들 정의
-------------------------------------*/
#define WorldMgr						WorldManager::GetInstance()
#define StatMgr							StatManager::GetInstance()



/*-------------------------------------
			스마트 포인터
-------------------------------------*/
SHARED_PTR(Player)
SHARED_PTR(GameObject)

UNIQUE_PTR(BaseZone)
UNIQUE_PTR(ZoneGrid)
