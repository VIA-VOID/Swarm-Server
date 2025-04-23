#include "pch.h"
#include "Service/GameServerManager.h"

class Player
{
public:
	void UseSkill(int skillId, int targetId)
	{
		LOG_INFO(L"Player used skill " + std::to_wstring(skillId) + L" on target");
	}

	void SkillCooldownComplete(int skillId)
	{
		LOG_INFO(L"Skill " + std::to_wstring(skillId) + L" cooldown complete, ready to use again!");
	}
};

class Dungeon
{
public:
	void SpawnMonster(int monsterId, int count)
	{
		LOG_INFO(L"Spawned " + std::to_wstring(count) + L" monsters of type " + std::to_wstring(monsterId));
	}
};

int wmain()
{
	GameServerMgr.Init();

	std::this_thread::sleep_for(std::chrono::seconds(1));

	JobQ.RegisterTypeMapping<Dungeon>(JobGroupType::Dungeon);
	JobQ.RegisterTypeMapping<Player>(JobGroupType::Player);

	auto player = ObjectPool<Player>::MakeShared();
	auto dungeon = ObjectPool<Dungeon>::MakeShared();

	/*
	JobQ.DoAsync([=]()
		{
		// 메모리풀 테스트
			Vector<int> v;
			for (int i = 0; i < 100; i++)
			{
				v.push_back(i);
				LOG_INFO(L" 1 "); std::this_thread::sleep_for(std::chrono::milliseconds(500));
			}
			v.clear();
		}
	);
	*/

	// 우선순위 테스트
	JobQ.DoAsync(dungeon, &Dungeon::SpawnMonster, 111, 10);
	JobQ.DoAsync(player, &Player::UseSkill, 1, 100);
	JobQ.DoAsync(dungeon, &Dungeon::SpawnMonster, 1, 2);
	JobQ.DoAsync(dungeon, &Dungeon::SpawnMonster, 3, 4);
	JobQ.DoAsync(player, &Player::UseSkill, 2, 200);

	std::this_thread::sleep_for(std::chrono::seconds(5));

	while (1)
	{
		JobQ.DoAsyncAfter(2000, player, &Player::SkillCooldownComplete, 3);
		JobQ.DoAsync(player, &Player::UseSkill, 1, 100);
		JobQ.DoAsyncAfter(2002, dungeon, &Dungeon::SpawnMonster, 5, 10);


		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}


	GameServerMgr.Shutdown();

	return 0;
}
