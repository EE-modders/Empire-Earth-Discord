#include "pch.h"
//#include "MonaLisas_CPP_Discord.h"
#include <Windows.h>
#include <string>
#include <vector>
#include <VersionHelpers.h>
#include <algorithm>
#include <tlhelp32.h>
#include <chrono>
#include <thread>

using namespace std;

int EE215ProcessID = 0;
int EE1_Zero_AOC_One = -1;
int CD_Zero_NeoEE_One = -1;
bool IsThisEE2 = true;
bool IsThisEE2X = false;
bool IsThisNeoEE = false;
bool AllowDiscordStatusThread = true;

bool KeepAliveGameStarted = true;

int L_Stored_Current_Game_Type_SP0_MP1 = 0;
int L_Number_of_Human_Players_in_MP_Game = -1;
int L_Stored_Current_Player_Type_Obs1 = 0;
int L_Stored_Game_Currently_In_Game_or_Menu_Lobby1 = -1;
int L_Stored_Currently_Really_in_MP_Lobby1 = -1;
int L_Stored_Current_Game_is_Playback1 = -1;
string L_Stored_Playback_Nickname_String_Std = "Nick";

bool ChatIsEnabled = false;//You don't have a support chat, so it should always be false.

bool Got_DISCORDGAMEAP = false;
//IMPORTANT. This contains a string with IDs and other info (Configuration from the server). Keep it this way, because you'll possibly need it this way in the future:
string DISCORDGAMEAP = "1;;;1;;;782679873856077914;;;783353615670706217;;;logo;;;Empire Earth;;;logo;;;Empire Earth: The Art of Conquest;;;NULL;;;NULL;;;NULL;;;NULL;;;Game is minimized...;;;Using the Support Chat;;;In Launcher;;;In Game Menu;;;In Multiplayer Lobby;;;In MP Lobby (minimized);;;Watching a playback;;;Observing (Singleplayer);;;Playing (Singleplayer);;;Observing (Multiplayer);;;Playing (Multiplayer);;;Starting game...;;;NULL";
vector<string> Discord_Game_API_Server_Config;

//END of EE1 configurable commands

long long DISCORD_CLIENT_APP_ID_EE2 = 0;
long long DISCORD_CLIENT_APP_ID_AOS = 0;
long long DISCORD_CLIENT_APP_ID_Current = 0;
bool MonaLisaDiscordThreadAlreadyRunning = false;
string Current_Discord_User_Name = "";
string Current_Discord_User_ID = "";

bool KeepAliveInterruptCurrentSleep = false;
//Don't worry about it. It's for KeepAlive thread that EE1 currently doesn't have

string DiscordStatusStringFromKeepAlive = "";
string PreviousDiscordStatusStringFromKeepAlive = "";
vector<string> DiscordStatusStringFromKeepAliveVector;



#include <csignal>
#include "discord_game_sdk/cpp/discord.h"
#include "discord_game_sdk/c/discord_game_sdk.h"
//#include "discord.h"'

#include "discord_game_sdk/cpp/achievement_manager.cpp"
#include "discord_game_sdk/cpp/activity_manager.cpp"
#include "discord_game_sdk/cpp/application_manager.cpp"
#include "discord_game_sdk/cpp/core.cpp"
#include "discord_game_sdk/cpp/image_manager.cpp"
#include "discord_game_sdk/cpp/lobby_manager.cpp"
#include "discord_game_sdk/cpp/network_manager.cpp"
#include "discord_game_sdk/cpp/overlay_manager.cpp"
#include "discord_game_sdk/cpp/relationship_manager.cpp"
#include "discord_game_sdk/cpp/storage_manager.cpp"
#include "discord_game_sdk/cpp/store_manager.cpp"
#include "discord_game_sdk/cpp/types.cpp"
#include "discord_game_sdk/cpp/user_manager.cpp"
#include "discord_game_sdk/cpp/voice_manager.cpp"

namespace {
    volatile bool Discord_interrupted{ false };
}

struct DiscordState {
    discord::User currentUser;
    std::unique_ptr<discord::Core> core;
};

int Discord_Initialization_Code = -1;

int OldEE2_or_AOS_status = -1;

string DiscordLastInGameStatusNow = "Playing";
string DiscordLastInGameStatusNowSecondLine = "";

bool DontSleepBecauseStatusChanged = false;

BOOL FileExists(LPCTSTR szPath)
{
	//In EE1 we don't use boost... yet?
	/*if (boost::filesystem::exists(szPath)) {
		return true;
	}
	return false;*/

	DWORD dwAttrib = GetFileAttributes(szPath);
	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

vector<string> split(const string& s, const string& delim, const bool keep_empty = true, std::string AddAtTheEnd = "", std::string AddAtTheTop = "", std::string AddDelimIfFoundStr = "") {
	//MessageBox(Message4, to_string(delim.length()).c_str(), "delim LEN", MB_OK | MB_ICONERROR);
	vector<string> result;
	if (delim.empty()) {
		result.push_back(s);
		return result;
	}
	string::const_iterator substart = s.begin(), subend, substartHotifx = s.begin();
	while (true) {
		subend = search(substart, s.end(), delim.begin(), delim.end());
		string temp(substart, subend);
		if (keep_empty || !temp.empty()) {
			if (AddAtTheEnd.length() > 0) {
				string temp2 = temp + AddAtTheEnd;
				//MessageBox(NULL, temp2.c_str(), to_string(AddDelimIfFoundStr.length()).c_str(), MB_OK);
				if (AddDelimIfFoundStr.length() > 0) {
					string tempFix(substartHotifx, subend);
					//MessageBox(NULL, tempFix.c_str(), "tempFix", MB_OK);
					if (tempFix.find(AddDelimIfFoundStr) == std::string::npos) {
						temp2 = temp;
						//MessageBox(NULL, temp2.c_str(), to_string(AddDelimIfFoundStr.length()).c_str(), MB_OK);
					}
				}
				result.push_back(temp2);
			}
			else if (AddAtTheTop.length() > 0) {
				string temp2 = AddAtTheTop + temp;
				if (AddDelimIfFoundStr.length() > 0) {
					string tempFix(substartHotifx, subend);
					//MessageBox(NULL, tempFix.c_str(), "tempFix", MB_OK);
					if (tempFix.find(AddDelimIfFoundStr) == std::string::npos) {
						temp2 = temp;
					}
				}
				result.push_back(temp2);
			}
			else {
				result.push_back(temp);
			}
		}
		if (subend == s.end()) {
			break;
		}
		substart = subend + delim.size();
		substartHotifx = subend;
	}
	return result;
}

BOOL IsProcessRunningPID(DWORD pid)
{
	HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, pid);
	DWORD ret = WaitForSingleObject(process, 0);
	CloseHandle(process);
	return ret == WAIT_TIMEOUT;
}

#include <string.h>
bool IsProcessRunningWs(const wstring processName) {
	bool exists = false;
	PROCESSENTRY32W entry;
	entry.dwSize = sizeof(PROCESSENTRY32W);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32FirstW(snapshot, &entry))
		while (Process32NextW(snapshot, &entry)) {

			//wcout << entry.szExeFile << endl;

			if (!_wcsicmp(entry.szExeFile, processName.c_str())) {

				exists = true;
				CloseHandle(snapshot);
				return true;
			}

		}

	CloseHandle(snapshot);
	return exists;
}

bool IsForegroundProcess(DWORD pid)
{
	HWND hwnd = GetForegroundWindow();
	if (hwnd == NULL) return false;

	DWORD foregroundPid;
	if (GetWindowThreadProcessId(hwnd, &foregroundPid) == 0) return false;

	return (foregroundPid == pid);
}

#include <ctime>
#include <chrono>
using namespace std::chrono;
bool EE2WindowIsForegroundNow = false;
milliseconds mbc_lastForegroundWindowCheck = milliseconds(0);
milliseconds mbc_msnow_extra_157031;
void UpdateIsForegroundProcessStatus(bool IgnoreSleep = false) {
	mbc_msnow_extra_157031 = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
	if (mbc_lastForegroundWindowCheck.count() + 1000 < mbc_msnow_extra_157031.count() || IgnoreSleep) {
		if (IsForegroundProcess(EE215ProcessID)) {
			EE2WindowIsForegroundNow = true;
		}
		else {
			EE2WindowIsForegroundNow = false;
		}
		mbc_lastForegroundWindowCheck = mbc_msnow_extra_157031;
	}
}

bool AllowDiscordStatusWhileInLauncher = true;

bool Mona_Discord_Status_Thread_ContinueTwitterNews = false;
int DiscordInitDontSleepTries = 0;

//hotfix:
bool JustStarted = false;//We dont use it due to discord bug.

void Mona_Discord_Status_Thread() {
	try {
		Discord_Game_API_Server_Config = split(DISCORDGAMEAP, ";;;");
		if (Discord_Game_API_Server_Config.size() >= 2) {
			Got_DISCORDGAMEAP = true;
		}
		else {
			Got_DISCORDGAMEAP = false;//Just in case that will never happen.
		}
		unsigned int SleepTimeDynamic = 2500;
		//MessageBox(NULL, "Mona_Discord_Status_Initialize()", "TRACK DISCORD INFO", MB_OK);
		if (AllowDiscordStatusThread) {
			if (IsWindows7OrGreater()) {
				if (FileExists(L"discord_game_sdk.dll")) {
					while (!Discord_interrupted && (Discord_Initialization_Code != 0)) {
						if (Got_DISCORDGAMEAP) {
							//MessageBox(NULL, L"Got_DISCORDGAMEAP()", L"TRACK DISCORD INFO", MB_OK);
							if (Discord_Game_API_Server_Config.size() >= 25) {
								//MessageBox(NULL, L"Discord_Game_API_Server_Config >= 25", L"TRACK DISCORD INFO", MB_OK);
								if (Discord_Game_API_Server_Config[0] == "1") {
									//MessageBox(NULL, L"Enabled by config", L"TRACK DISCORD INFO", MB_OK);
									if (IsProcessRunningWs(L"Discord.exe") || IsProcessRunningWs(L"discord.exe")) {

										//MessageBox(NULL, L"DISCORD RUNNING!!", L"TRACK DISCORD INFO", MB_OK);
										DiscordInitDontSleepTries = 0;
										SleepTimeDynamic = 4000;
										//Resetting:
										DiscordLastInGameStatusNow = "";
										DiscordLastInGameStatusNowSecondLine = "";

										bool UpdateLocalDetails = true;
										bool ShowMultiplayerName = false;
										bool ShownSingleplayerGameTimer = false;
										if (Discord_Game_API_Server_Config[1] == "1") {
											ShowMultiplayerName = true;
										}

										DISCORD_CLIENT_APP_ID_EE2 = atoll(Discord_Game_API_Server_Config[2].c_str());
										DISCORD_CLIENT_APP_ID_AOS = atoll(Discord_Game_API_Server_Config[3].c_str());

										//MessageBoxA(NULL, to_string(DISCORD_CLIENT_APP_ID_EE2).c_str(), "Client API ID of EE1 Normal:", MB_OK);
										//MessageBoxA(NULL, to_string(DISCORD_CLIENT_APP_ID_AOS).c_str(), "Client API ID of EE1 AOC:", MB_OK);


										string SetLargeImageDefault = "";
										string SetLargeTextDefault = "";
										string SetSmallImageDefault = "";
										string SetSmallTextDefault = "";
										string SetDetailsDefault = "";
										string SetStateDefault = "";
										long long SetStartDefault = 0;
										long long SetEndDefault = 0;

										if (IsThisEE2) {
											OldEE2_or_AOS_status = 0;
											if (Discord_Game_API_Server_Config[4] != "NULL") {
												SetLargeImageDefault = Discord_Game_API_Server_Config[4].c_str();
											}
											if (Discord_Game_API_Server_Config[5] != "NULL") {
												SetLargeTextDefault = Discord_Game_API_Server_Config[5].c_str();
											}
											if (Discord_Game_API_Server_Config[8] != "NULL") {
												SetSmallImageDefault = Discord_Game_API_Server_Config[8].c_str();
											}
											if (Discord_Game_API_Server_Config[9] != "NULL") {
												SetSmallTextDefault = Discord_Game_API_Server_Config[9].c_str();
											}
										}
										else if (IsThisEE2X) {
											OldEE2_or_AOS_status = 1;
											if (Discord_Game_API_Server_Config[6] != "NULL") {
												SetLargeImageDefault = Discord_Game_API_Server_Config[6].c_str();
											}
											if (Discord_Game_API_Server_Config[7] != "NULL") {
												SetLargeTextDefault = Discord_Game_API_Server_Config[7].c_str();
											}
											if (Discord_Game_API_Server_Config[10] != "NULL") {
												SetSmallImageDefault = Discord_Game_API_Server_Config[10].c_str();
											}
											if (Discord_Game_API_Server_Config[11] != "NULL") {
												SetSmallTextDefault = Discord_Game_API_Server_Config[11].c_str();
											}
										}

										string MinimizedStr_l = "Game is minimized...";
										size_t curConfigNum = 12;
										if (Discord_Game_API_Server_Config[curConfigNum] != "NULL") {
											MinimizedStr_l = Discord_Game_API_Server_Config[curConfigNum];
										}
										string onSupportChatStr_l = "Using the Support Chat";
										curConfigNum++;
										if (Discord_Game_API_Server_Config[curConfigNum] != "NULL") {
											onSupportChatStr_l = Discord_Game_API_Server_Config[curConfigNum];
										}
										string inLauncher_l = "In Launcher";
										curConfigNum++;
										if (Discord_Game_API_Server_Config[curConfigNum] != "NULL") {
											inLauncher_l = Discord_Game_API_Server_Config[curConfigNum];
										}
										string inGameMenu_l = "In Game Menu";
										curConfigNum++;
										if (Discord_Game_API_Server_Config[curConfigNum] != "NULL") {
											inGameMenu_l = Discord_Game_API_Server_Config[curConfigNum];
										}
										string inMpLobby_l = "In Multiplayer Lobby";
										curConfigNum++;
										if (Discord_Game_API_Server_Config[curConfigNum] != "NULL") {
											inMpLobby_l = Discord_Game_API_Server_Config[curConfigNum];
										}
										string inMpLobbyMini_l = "In MP Lobby (minimized)";
										curConfigNum++;
										if (Discord_Game_API_Server_Config[curConfigNum] != "NULL") {
											inMpLobbyMini_l = Discord_Game_API_Server_Config[curConfigNum];
										}
										string watchingAPlayback_l = "Watching a playback";
										curConfigNum++;
										if (Discord_Game_API_Server_Config[curConfigNum] != "NULL") {
											watchingAPlayback_l = Discord_Game_API_Server_Config[curConfigNum];
										}
										string obsSingleplayer_l = "Observing (Singleplayer)";
										curConfigNum++;
										if (Discord_Game_API_Server_Config[curConfigNum] != "NULL") {
											obsSingleplayer_l = Discord_Game_API_Server_Config[curConfigNum];
										}
										string playingSinglePlayer_l = "Playing (Singleplayer)";
										curConfigNum++;
										if (Discord_Game_API_Server_Config[curConfigNum] != "NULL") {
											playingSinglePlayer_l = Discord_Game_API_Server_Config[curConfigNum];
										}
										string obsMultiplayer_l = "Observing (Multiplayer)";
										curConfigNum++;
										if (Discord_Game_API_Server_Config[curConfigNum] != "NULL") {
											obsMultiplayer_l = Discord_Game_API_Server_Config[curConfigNum];
										}
										string playingMultiplayer_l = "Observing (Multiplayer)";
										curConfigNum++;
										if (Discord_Game_API_Server_Config[curConfigNum] != "NULL") {
											playingMultiplayer_l = Discord_Game_API_Server_Config[curConfigNum];
										}
										string startingGame_l = "Starting game...";
										curConfigNum++;
										if (Discord_Game_API_Server_Config[curConfigNum] != "NULL") {
											startingGame_l = Discord_Game_API_Server_Config[curConfigNum];
										}
										string NameStr_l = "";
										curConfigNum++;
										if (Discord_Game_API_Server_Config[curConfigNum] != "NULL") {
											NameStr_l = Discord_Game_API_Server_Config[curConfigNum];
										}

										//Init:
										//MessageBox(NULL, "Init", "TRACK DISCORD INFO", MB_OK);
										DiscordState Discord_state{};
										discord::Core* Discord_core{};
										discord::Activity Discord_activity{};

										if (IsThisEE2) {
											DISCORD_CLIENT_APP_ID_Current = DISCORD_CLIENT_APP_ID_EE2;
										}
										else if (IsThisEE2X) {
											DISCORD_CLIENT_APP_ID_Current = DISCORD_CLIENT_APP_ID_AOS;
										}

										auto Discord_result = discord::Core::Create(DISCORD_CLIENT_APP_ID_Current, DiscordCreateFlags_NoRequireDiscord, &Discord_core);
										Discord_state.core.reset(Discord_core);
										if (Discord_state.core) {

											Discord_core->UserManager().OnCurrentUserUpdate.Connect([&Discord_state]() {
												Discord_state.core->UserManager().GetCurrentUser(&Discord_state.currentUser);
												Current_Discord_User_Name = Discord_state.currentUser.GetUsername();
												Current_Discord_User_Name = Current_Discord_User_Name + "#";
												Current_Discord_User_Name = Current_Discord_User_Name + Discord_state.currentUser.GetDiscriminator();
												//MessageBoxA(NULL, Current_Discord_User_Name.c_str(), "Current_Discord_User_Name:", MB_OK);

												Current_Discord_User_ID = to_string(Discord_state.currentUser.GetId());
												//MessageBox(NULL, Current_Discord_User_ID.c_str(), "Current_Discord_user_ID:", MB_OK);

												Discord_Initialization_Code = 1;
												//std::cout << "Current user updated: " << state.currentUser.GetUsername() << "#" << state.currentUser.GetDiscriminator() << "\n";

												KeepAliveInterruptCurrentSleep = true;//So we update KA with Discord info
												Mona_Discord_Status_Thread_ContinueTwitterNews = true;

												});

											Discord_activity.SetType(discord::ActivityType::Playing);
											//activity.SetType(discord::ActivityType::Watching);
											Discord_activity.GetAssets().SetLargeImage(SetLargeImageDefault.c_str());
											Discord_activity.GetAssets().SetLargeText(SetLargeTextDefault.c_str());
											Discord_activity.GetAssets().SetSmallImage(SetSmallImageDefault.c_str());
											Discord_activity.GetAssets().SetSmallText(SetSmallTextDefault.c_str());
											Discord_activity.SetDetails(SetDetailsDefault.c_str());
											Discord_activity.SetState(SetStateDefault.c_str());
											Discord_activity.GetTimestamps().SetStart(SetStartDefault);
											Discord_activity.GetTimestamps().SetEnd(SetEndDefault);

											//158005 Check for AllowDiscordStatusWhileInLauncher
											while (!KeepAliveGameStarted && !AllowDiscordStatusWhileInLauncher && !Discord_interrupted) {
												std::this_thread::sleep_for(std::chrono::milliseconds(1000));
											}

											Discord_state.core->ActivityManager().UpdateActivity(Discord_activity, [](discord::Result Discord_result) {
												//std::cout << ((result == discord::Result::Ok) ? "Succeeded" : "Failed") << " updating activity!\n";
												if (Discord_result == discord::Result::Ok) {
													//MessageBoxA(NULL, "Successfully updated discord activity", "Discord Game API:", MB_OK);
												}
												else {
													//string ErrorMessage = "Failed updating Discord Activity! (err " + to_string(static_cast<int>(Discord_result)) + ")";
													//MessageBoxA(NULL, ErrorMessage.c_str(), "Discord Game API:", MB_OK);
												}
												});

											do {
												//Change Game App

												bool EnableAdvancedStatusCheck = true;//In EE1 we now use modified version of it. So keep enabled..
												if (EnableAdvancedStatusCheck) {

													if (IsThisEE2 && OldEE2_or_AOS_status == 1) {
														Discord_Initialization_Code = -1;
														DontSleepBecauseStatusChanged = true;
														break;
													}
													else if (IsThisEE2X && OldEE2_or_AOS_status == 0) {
														Discord_Initialization_Code = -1;
														DontSleepBecauseStatusChanged = true;
														break;
													}

													bool UpdateStatus = false;

													string InGameStatusNow = "";
													bool Observing = false;
													string InGameStatusNowSecondLine = "";
													bool CurrentlySPorWatchingPlayback = false;

													//Currently disabled on EE1, as there is no data from PHP script:
													
													/*if (DiscordStatusStringFromKeepAlive != PreviousDiscordStatusStringFromKeepAlive) {
														DiscordStatusStringFromKeepAliveVector = split(DiscordStatusStringFromKeepAlive, ";;;");
														size_t NumberOfConfigurableValues = 8;
														if (DiscordStatusStringFromKeepAliveVector.size() >= NumberOfConfigurableValues) {

															//0 = SetLargeImage
															//1 = SetLargeText
															//2 = SetSmallImage
															//3 = SetSmallText
															//4 = SetDetails
															//5 = SetState
															//6 = SetStart
															//7 = SetEnd

															if (DiscordStatusStringFromKeepAliveVector[0] != "NULL") {
																if (DiscordStatusStringFromKeepAliveVector[0] == "EMPTY") {
																	Discord_activity.GetAssets().SetLargeImage("");
																}
																else if (DiscordStatusStringFromKeepAliveVector[0] == "DEFAULT") {
																	Discord_activity.GetAssets().SetLargeImage(SetLargeImageDefault.c_str());
																}
																else {
																	Discord_activity.GetAssets().SetLargeImage(DiscordStatusStringFromKeepAliveVector[0].c_str());
																}
																UpdateStatus = true;

															}
															if (DiscordStatusStringFromKeepAliveVector[1] != "NULL") {
																if (DiscordStatusStringFromKeepAliveVector[1] == "EMPTY") {
																	Discord_activity.GetAssets().SetLargeText("");
																}
																else if (DiscordStatusStringFromKeepAliveVector[1] == "DEFAULT") {
																	Discord_activity.GetAssets().SetLargeText(SetLargeTextDefault.c_str());
																}
																else {
																	Discord_activity.GetAssets().SetLargeText(DiscordStatusStringFromKeepAliveVector[1].c_str());
																}
																UpdateStatus = true;

															}
															if (DiscordStatusStringFromKeepAliveVector[2] != "NULL") {
																if (DiscordStatusStringFromKeepAliveVector[2] == "EMPTY") {
																	Discord_activity.GetAssets().SetSmallImage("");
																}
																else if (DiscordStatusStringFromKeepAliveVector[2] == "DEFAULT") {
																	Discord_activity.GetAssets().SetSmallImage(SetSmallImageDefault.c_str());
																}
																else {
																	Discord_activity.GetAssets().SetSmallImage(DiscordStatusStringFromKeepAliveVector[2].c_str());
																}
																UpdateStatus = true;
															}
															if (DiscordStatusStringFromKeepAliveVector[3] != "NULL") {
																if (DiscordStatusStringFromKeepAliveVector[3] == "EMPTY") {
																	Discord_activity.GetAssets().SetSmallText("");
																}
																else if (DiscordStatusStringFromKeepAliveVector[3] == "DEFAULT") {
																	Discord_activity.GetAssets().SetSmallText(SetSmallTextDefault.c_str());
																}
																else {
																	Discord_activity.GetAssets().SetSmallText(DiscordStatusStringFromKeepAliveVector[3].c_str());
																}
																UpdateStatus = true;
															}
															if (DiscordStatusStringFromKeepAliveVector[4] != "NULL") {
																if (DiscordStatusStringFromKeepAliveVector[4] == "EMPTY") {
																	UpdateLocalDetails = false;
																	Discord_activity.SetDetails("");
																}
																else if (DiscordStatusStringFromKeepAliveVector[4] == "DEFAULT") {
																	UpdateLocalDetails = true;
																	InGameStatusNow = SetDetailsDefault;
																	DiscordLastInGameStatusNow = SetDetailsDefault;
																	Discord_activity.SetDetails(SetDetailsDefault.c_str());
																}
																else {
																	UpdateLocalDetails = false;
																	Discord_activity.SetDetails(DiscordStatusStringFromKeepAliveVector[4].c_str());
																}
																UpdateStatus = true;
															}
															else {
																UpdateLocalDetails = true;
																InGameStatusNow = "";
																DiscordLastInGameStatusNow = "";
															}

															if (DiscordStatusStringFromKeepAliveVector[5] != "NULL") {
																if (DiscordStatusStringFromKeepAliveVector[5] == "EMPTY") {
																	Discord_activity.SetState("");
																	InGameStatusNowSecondLine = "";
																	DiscordLastInGameStatusNowSecondLine = "";
																}
																else if (DiscordStatusStringFromKeepAliveVector[5] == "DEFAULT") {
																	Discord_activity.SetState(SetStateDefault.c_str());

																	InGameStatusNowSecondLine = SetStateDefault;
																	DiscordLastInGameStatusNowSecondLine = SetStateDefault;
																}
																else {
																	Discord_activity.SetState(DiscordStatusStringFromKeepAliveVector[5].c_str());
																	InGameStatusNowSecondLine = DiscordStatusStringFromKeepAliveVector[5];
																	DiscordLastInGameStatusNowSecondLine = DiscordStatusStringFromKeepAliveVector[5];
																}
																UpdateStatus = true;
															}
															if (DiscordStatusStringFromKeepAliveVector[6] != "NULL") {
																if (DiscordStatusStringFromKeepAliveVector[6] == "EMPTY") {
																	Discord_activity.GetTimestamps().SetStart(0);
																}
																else if (DiscordStatusStringFromKeepAliveVector[6] == "DEFAULT") {
																	Discord_activity.GetTimestamps().SetStart(SetStartDefault);
																}
																else {
																	Discord_activity.GetTimestamps().SetStart(atoll(DiscordStatusStringFromKeepAliveVector[6].c_str()));
																}
																UpdateStatus = true;
															}
															if (DiscordStatusStringFromKeepAliveVector[7] != "NULL") {
																if (DiscordStatusStringFromKeepAliveVector[7] == "EMPTY") {
																	Discord_activity.GetTimestamps().SetEnd(0);
																}
																else if (DiscordStatusStringFromKeepAliveVector[7] == "DEFAULT") {
																	Discord_activity.GetTimestamps().SetEnd(SetEndDefault);
																}
																else {
																	Discord_activity.GetTimestamps().SetEnd(atoll(DiscordStatusStringFromKeepAliveVector[7].c_str()));
																}
																UpdateStatus = true;
															}
														}

														PreviousDiscordStatusStringFromKeepAlive = DiscordStatusStringFromKeepAlive;
													}*/

													if (UpdateLocalDetails) {
														//EE1 special functions:

														InGameStatusNow = ""; //Clear just in case.
														//Note, second line is used by the timer, so we shouldn't tweak it.

														if (JustStarted) {
															JustStarted = false;
															InGameStatusNow = "Starting game...";
														}
														else {
															//MessageBoxA(NULL, "Updating status for the second time", "Fuck", MB_OK);

															if (CD_Zero_NeoEE_One == -1) {
																//This generally is an error that version detection failed, so keep it this way, but it should never occur.
																InGameStatusNow = "Playing Unknown";
															}
															else if (CD_Zero_NeoEE_One == 0) {
																InGameStatusNow = "Playing";
															}
															else if (CD_Zero_NeoEE_One == 1) {
																InGameStatusNow = "Playing (NeoEE Patched)";
															}

															if (EE215ProcessID != 0) {
																UpdateIsForegroundProcessStatus();
																if (!EE2WindowIsForegroundNow) {
																	InGameStatusNow = InGameStatusNow + " (Minimized)";
																}
															}
														}

														//Show game timer since start:
														CurrentlySPorWatchingPlayback = true;


														/*string NameStrNow_l = NameStr_l + L_Stored_Playback_Nickname_String_Std;
														if (!KeepAliveGameStarted) {
															InGameStatusNow = inLauncher_l;
															if (ChatIsEnabled) {
																InGameStatusNowSecondLine = onSupportChatStr_l;
																//InGameStatusNow = "Launcher (Support Chat)";
															}
															else {
																InGameStatusNowSecondLine = "";
															}
														}
														else {
															if (L_Stored_Game_Currently_In_Game_or_Menu_Lobby1 == 1) {
																InGameStatusNow = inGameMenu_l;
																if (L_Stored_Currently_Really_in_MP_Lobby1 == 1) {
																	InGameStatusNow = inMpLobby_l;
																	UpdateIsForegroundProcessStatus();
																	if (EE2WindowIsForegroundNow) {
																		if (ShowMultiplayerName && (L_Stored_Playback_Nickname_String_Std.length() > 2)) {
																			InGameStatusNowSecondLine = NameStrNow_l;
																		}
																		else {
																			InGameStatusNowSecondLine = "";

																		}

																	}
																	else {
																		if (ShowMultiplayerName && (L_Stored_Playback_Nickname_String_Std.length() > 2)) {
																			InGameStatusNow = inMpLobbyMini_l;
																			InGameStatusNowSecondLine = NameStrNow_l;
																		}
																		else {
																			InGameStatusNowSecondLine = MinimizedStr_l;
																		}
																	}

																}
															}
															else if (L_Stored_Game_Currently_In_Game_or_Menu_Lobby1 == 0) {
																if (L_Stored_Current_Game_is_Playback1 == 1) {
																	CurrentlySPorWatchingPlayback = true;
																	InGameStatusNow = watchingAPlayback_l;
																}
																else if (L_Stored_Current_Game_Type_SP0_MP1 == 0) {
																	CurrentlySPorWatchingPlayback = true;
																	if (L_Stored_Current_Player_Type_Obs1 == 1) {
																		InGameStatusNow = obsSingleplayer_l;
																		Observing = true;
																	}
																	else {
																		InGameStatusNow = playingSinglePlayer_l;
																	}
																	//Here was bug with else if, because we dont get obs status in SP.
																}
																else if (L_Stored_Current_Game_Type_SP0_MP1 == 1) {
																	if (L_Stored_Current_Player_Type_Obs1 == 1) {
																		InGameStatusNow = obsMultiplayer_l;
																		Observing = true;
																		if (ShowMultiplayerName && L_Stored_Playback_Nickname_String_Std.length() > 2) {
																			//InGameStatusNow = "Observing (Multiplayer). Name: " + L_Stored_Playback_Nickname_String_Std;
																			InGameStatusNow = obsMultiplayer_l;
																		}
																	}
																	else if (L_Stored_Current_Player_Type_Obs1 == 0) {
																		InGameStatusNow = playingMultiplayer_l;
																		if (ShowMultiplayerName && L_Stored_Playback_Nickname_String_Std.length() > 2) {
																			//InGameStatusNow = "Playing (Multiplayer). Name: " + L_Stored_Playback_Nickname_String_Std;
																			InGameStatusNow = playingMultiplayer_l;
																		}
																	}
																	else {
																		InGameStatusNow = startingGame_l;
																	}
																}
																else {
																	InGameStatusNow = startingGame_l;
																}
															}
															else {
																InGameStatusNow = startingGame_l;
															}
														}*/


														if (DiscordLastInGameStatusNowSecondLine != InGameStatusNowSecondLine) {
															//hotfix lobby minimize status:
															//if (DiscordLastInGameStatusNowSecondLine == "" || DiscordLastInGameStatusNowSecondLine == NameStrNow_l || DiscordLastInGameStatusNowSecondLine == MinimizedStr_l || DiscordLastInGameStatusNowSecondLine == onSupportChatStr_l) {
															
																Discord_activity.SetState(InGameStatusNowSecondLine.c_str());
																UpdateStatus = true;
																DiscordLastInGameStatusNowSecondLine = InGameStatusNowSecondLine;
															//}
														}

														if (DiscordLastInGameStatusNow != InGameStatusNow) {
															//MessageBoxA(NULL, InGameStatusNow.c_str(), "New status should update now:", MB_OK);
															Discord_activity.SetDetails(InGameStatusNow.c_str());
															UpdateStatus = true;
															DiscordLastInGameStatusNow = InGameStatusNow;
														}


													}

													if (UpdateStatus) {
														if (CurrentlySPorWatchingPlayback) {
															if (!ShownSingleplayerGameTimer) {
																Discord_activity.GetTimestamps().SetStart(std::time(nullptr));
																ShownSingleplayerGameTimer = true;
															    //MessageBoxA(NULL, "Updated Discord Status", "Result::Ok", MB_OK);
															}
														}
														else {
															if (ShownSingleplayerGameTimer) {
																Discord_activity.GetTimestamps().SetStart(0);
																ShownSingleplayerGameTimer = false;
															}
														}
														//MessageBox(NULL, "Status updatr request", "Status updatr request", MB_OK);
														Discord_state.core->ActivityManager().UpdateActivity(Discord_activity, [](discord::Result Discord_result) {
															if (Discord_result == discord::Result::Ok) {
																//MessageBox(NULL, "Result::Ok", "Result::Ok", MB_OK);
															}
															else {
																//string ErrorMessage = "Failed updating Discord Activity! (err " + to_string(static_cast<int>(Discord_result)) + ")";
																//MessageBoxA(NULL, ErrorMessage.c_str(), "Discord Game API:", MB_OK);
															}
															});
													}

												}

												Discord_result = Discord_state.core->RunCallbacks();

												if (Discord_result == discord::Result::NotRunning) {
													DontSleepBecauseStatusChanged = true;
													//MessageBoxA(NULL, "discord::Result::NotRunning", "discord::Result::NotRunning", MB_OK);
													break;
													
												}
												//It never has NotInstalled, but keep it anyway:
												else if (Discord_result == discord::Result::NotInstalled) {
													Mona_Discord_Status_Thread_ContinueTwitterNews = true;
													Discord_Initialization_Code = 0;
													//MessageBox(NULL, "discord::Result::NotInstalled", "discord::Result::NotInstalled", MB_OK);
													break;
												}


												//Discord_state.core->RunCallbacks();
												//MessageBox(NULL, "After Callbacks", "After Callbacks", MB_OK);
												//std::this_thread::sleep_for(std::chrono::milliseconds(16));

												if (!Mona_Discord_Status_Thread_ContinueTwitterNews && (DiscordInitDontSleepTries < 10)) {
													DiscordInitDontSleepTries++;
													std::this_thread::sleep_for(std::chrono::milliseconds(16));
												}
												else {
													std::this_thread::sleep_for(std::chrono::milliseconds(1000));
													//MessageBoxA(NULL, "After longer sleep", "After Callbacks", MB_OK);
												}
											} while (!Discord_interrupted);

										}
										else {
											Mona_Discord_Status_Thread_ContinueTwitterNews = true;
											//MessageBox(NULL, "!Discord_state.core", "ERROR", MB_OK);
										}

										//MessageBox(NULL, "After UserManager()", "TRACK DISCORD INFO", MB_OK);

									}
									else {
										Mona_Discord_Status_Thread_ContinueTwitterNews = true;
										//Sleep 30s to recheck if process is running
										SleepTimeDynamic = 30000;
										// Discord.exe not running, wait
									}
								}
								else if (Discord_Game_API_Server_Config[0] == "0") {
									Mona_Discord_Status_Thread_ContinueTwitterNews = true;
									Discord_Initialization_Code = 0;
								}
							}
							else {
								Mona_Discord_Status_Thread_ContinueTwitterNews = true;
								Discord_Initialization_Code = 0;
							}

						}
						else {
							Mona_Discord_Status_Thread_ContinueTwitterNews = true;
							Discord_Initialization_Code = 0;
						}
						//Sleep
						if (!DontSleepBecauseStatusChanged) {
							std::this_thread::sleep_for(std::chrono::milliseconds(SleepTimeDynamic));
							//Sleep(2500);
							//Sleep(SleepTimeDynamic);
						}
						else {
							DontSleepBecauseStatusChanged = false;
						}
						Mona_Discord_Status_Thread_ContinueTwitterNews = true;
					}
				}
				else {
					Mona_Discord_Status_Thread_ContinueTwitterNews = true;
					Discord_Initialization_Code = 0;
				}
			}
			else {
				Mona_Discord_Status_Thread_ContinueTwitterNews = true;
				Discord_Initialization_Code = 0;
			}
		}
		//MessageBox(NULL, "DISCORD THREAD EXIT / INTERRUPTED", "TRACK DISCORD INFO", MB_OK);
	}
	catch (...) {
		//MessageBox(NULL, "EXCEPTION IN DISCORD SDK", "EXCEPTION IN DISCORD SDK", MB_OK);
		Mona_Discord_Status_Thread_ContinueTwitterNews = true;
	}
	Mona_Discord_Status_Thread_ContinueTwitterNews = true;
}


DWORD WINAPI DoStuff(LPVOID lpParameter)
{
	//MessageBoxW(NULL, L"Starting Mona_Discord_Status_Thread()", L"Starting Mona_Discord_Status_Thread()", MB_OK | MB_ICONINFORMATION);
	Mona_Discord_Status_Thread();
    // The new thread will start here
    return 0;
}


DWORD WINAPI Thread_no_66(LPVOID lpParam)
{
	int     Data66 = 0;
	int     count66 = 0;
	HANDLE  hStdout66 = NULL;

	if ((hStdout66 =
		GetStdHandle(STD_OUTPUT_HANDLE))
		== INVALID_HANDLE_VALUE)
		return 1;
	Data66 = *((int*)lpParam);

	for (count66 = 0; count66 <= 0; count66++) //0 how many times repeat. 0 = 1
	{
		Mona_Discord_Status_Thread();
	}
	return 0;
}

void StartThread66() {
	//MessageBox(NULL, "sss", _T(thiswindowtitle6.c_str()), MB_YESNO | MB_ICONQUESTION);
	int Data_Of_Thread_66 = 1;
	HANDLE Handle_Of_Thread_66 = 0;
	HANDLE Array_Of_Thread_Handles66[1];
	Handle_Of_Thread_66 = CreateThread(NULL, 0,
		Thread_no_66, &Data_Of_Thread_66, 0, NULL);
	if (Handle_Of_Thread_66 == NULL)
		ExitProcess(Data_Of_Thread_66);
	Array_Of_Thread_Handles66[0] = Handle_Of_Thread_66;
}

unsigned int __stdcall mythreadA(void* data)
{
	Mona_Discord_Status_Thread();
	return 0;
}

//#include <boost/thread.hpp>

#include <shlwapi.h>

std::wstring base_name_ws(std::wstring const& path)
{
	return path.substr(path.find_last_of(L"/\\") + 1);
}

void Start_Mona_Lisa_Discord_Thread() {
	if (!MonaLisaDiscordThreadAlreadyRunning) {
		EE215ProcessID = GetCurrentProcessId();//So it knows the pid of EE1 process.

		//Check if game is EE1 or EE1: AOC:
		wchar_t CurrentWorkingDirectoryWhereExeIs[MAX_PATH];
		std::wstring ress = std::wstring(CurrentWorkingDirectoryWhereExeIs, GetModuleFileNameW(NULL, CurrentWorkingDirectoryWhereExeIs, MAX_PATH));

		std::wstring CurrentExeFileNameOnly = base_name_ws(CurrentWorkingDirectoryWhereExeIs);

		PathRemoveFileSpecW(CurrentWorkingDirectoryWhereExeIs);
		std::wstring CurrentExeWorksPath = CurrentWorkingDirectoryWhereExeIs;

		std::wstring CurrentExeFileNameOnlyLowercase = CurrentExeFileNameOnly;

		std::transform(CurrentExeFileNameOnlyLowercase.begin(), CurrentExeFileNameOnlyLowercase.end(), CurrentExeFileNameOnlyLowercase.begin(), ::tolower);


		//MessageBoxW(NULL, CurrentExeFileNameOnlyLowercase.c_str(), L"Lowercase exe name is", MB_YESNO | MB_ICONQUESTION);

		if (CurrentExeFileNameOnlyLowercase.find(L"ee-aoc.exe") != std::wstring::npos) {
			EE1_Zero_AOC_One = 1;
			IsThisEE2 = false;
			IsThisEE2X = true;
			//MessageBoxW(NULL, L"EE1 AOC", L"DETECTED", MB_YESNO | MB_ICONQUESTION);
		}
		else if (CurrentExeFileNameOnlyLowercase.find(L"empire earth.exe") != std::wstring::npos) {
			EE1_Zero_AOC_One = 0;
			IsThisEE2 = true;
			IsThisEE2X = false;
			//MessageBoxW(NULL, L"EE1 Normal", L"DETECTED", MB_YESNO | MB_ICONQUESTION);
		}

		if (FileExists(L"neoee.dll")) {
			CD_Zero_NeoEE_One = 1;
			IsThisNeoEE = true;
		}
		else {
			CD_Zero_NeoEE_One = 0;
			IsThisNeoEE = false;
		}


		HANDLE myhandleA, myhandleB;

		myhandleA = (HANDLE)_beginthreadex(0, 0, &mythreadA, 0, 0, 0);
		//myhandleB = (HANDLE)_beginthreadex(0, 0, &mythreadB; , 0, 0, 0);
		//WaitForSingleObject(myhandleA, INFINITE);
		//WaitForSingleObject(myhandleB, INFINITE);
		//CloseHandle(myhandleA);
		//CloseHandle(myhandleB);

		return;

		//boost::thread FuckIt(Mona_Discord_Status_Thread);

		return;

		StartThread66();
		return;

		//MessageBoxW(NULL, L"Before hThread", L"Before hThread", MB_OK | MB_ICONINFORMATION);

		//std::thread thread(Mona_Discord_Status_Thread);

		//return;

		DWORD ThreadId;
		HANDLE ThreadHandle;
		int Param = 0;
		/* create the thread */
		ThreadHandle = CreateThread(NULL,  0, DoStuff,  &Param,  0, &ThreadId);
		if (ThreadHandle == NULL) {
			//MessageBoxW(NULL, L"hThread == NULL :(", L"hThread == NULL :(", MB_OK | MB_ICONINFORMATION);
		}

		//MessageBoxW(NULL, L"After hThread", L"After hThread", MB_OK | MB_ICONINFORMATION);

        /*
        // Wait for thread to finish execution
        WaitForSingleObject(ThreadHandle, INFINITE);

        // Thread handle must be closed when no longer needed
        CloseHandle(ThreadHandle);
        */
	}
}