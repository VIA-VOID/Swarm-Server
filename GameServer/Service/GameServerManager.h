#pragma once

/*-------------------------------------------------------
				GameServerManager
--------------------------------------------------------*/

class GameServerManager : public Singleton<GameServerManager>
{
public:
	void Init() override;
	void Shutdown() override;

};

