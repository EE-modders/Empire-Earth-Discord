#pragma once
#pragma once

#include <string>
#include <map>

class GameQuery
{
public:

	GameQuery();

	enum ProductType {
		PT_Unknown,
		PT_EE,
		PT_AoC
	};

	enum ScreenType {
		ST_Unknown,
		ST_Menu,
		ST_PlayingSolo,
		ST_PlayingOnline,
		ST_Lobby,
		ST_ScenarioEditor		// Can't be detected
	};

	bool isLoaded();
	bool isPlaying();
	bool inLobby();

	bool isMinimized(); // WARNING: The debug console is also considered as the game Window

	bool isNeoEE();

	const char* getUsername();

	ScreenType getCurrentScreen();

	const char* getGameBaseVersion();
	const char* getGameDataVersion();
	bool isSupportedVersion();

	ProductType getProductType();

private:
	std::string _game_path;
	ProductType _productType;
	bool _neoee;

};
