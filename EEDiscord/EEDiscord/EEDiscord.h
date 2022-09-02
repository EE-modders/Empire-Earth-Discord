#pragma once

#include "Utils.h"
#include "Logger.h"
#include "GameQuery.h"

#include "discord-files/discord.h"

#include <string>

// Project INFO
static const std::string PROJECT_NAME_STR = "EEDiscord";
static const std::string PROJECT_AUTHOR_STR = "EnergyCube";
static const std::string PROJECT_VERSION_STR = "1.1.1";
static const unsigned int PROJECT_VERSION_MAJOR = 1;
static const unsigned int PROJECT_VERSION_MINOR = 1;
static const unsigned int PROJECT_VERSION_PATCH = 1;
// END Project INFO

// Hard Coded Project Settings
static bool KEEP_ALIVE = true;		// Use => true:		the DLL will wait until game stop to call onStop() (Detach)
									//     => false:	the DLL will call onStop() if onStart() ended and didn't
									//					created any additional thread (so, if the DLL is fully inative)

class EEDiscord
{
public:
	void printCredit();

	void onStart();
	void onStop();

	void updatePresenceThread();
	void updateCallbackThread();

private:
	std::unique_ptr<GameQuery> _gq = std::make_unique<GameQuery>();

	struct DiscordState {
		discord::User currentUser;

		std::unique_ptr<discord::Core> core;
	};

	discord::Core* _core{};
	DiscordState _state{};
	discord::Activity _activity{};

	std::mutex _mtx;
	bool _stop = false;
};

