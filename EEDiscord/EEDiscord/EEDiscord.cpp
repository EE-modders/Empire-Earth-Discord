#include "pch.h"
#include "EEDiscord.h"

#include <cstdio>
#include <windows.h>
#include <tlhelp32.h>

void EEDiscord::printCredit()
{
    Logger::showMessage("------------------------------------------------------");
#ifdef _DEBUG
    Logger::showMessage("  " + PROJECT_NAME_STR + " v" + PROJECT_VERSION_STR + " (DEBUG)");
#else
    Logger::showMessage("  " + PROJECT_NAME_STR + " v" + PROJECT_VERSION_STR);
#endif // _DEBUG
    Logger::showMessage("  By " + PROJECT_AUTHOR_STR);
    Logger::showMessage("------------------------------------------------------");
}

bool IsProcessRunning(const wchar_t* processName)
{
    bool exists = false;
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    if (Process32First(snapshot, &entry)) {
        while (Process32Next(snapshot, &entry)) {
            if (!_wcsicmp(entry.szExeFile, processName)) {
                exists = true;
                break;
            }
        }
    }

    CloseHandle(snapshot);
    return exists;
}

std::string buildDetailString(GameQuery gq)
{
    std::stringstream ss;
    ss << "";

    if (gq.getProductType() != GameQuery::PT_EE || !gq.isSupportedVersion()) {
        // Nothing for the moment :<
    }
    else {
        if (gq.isPlaying())
            ss << "As \"" << gq.getUsername() << "\"";
    }
    return ss.str();
}


std::string buildStateString(GameQuery gq)
{
    std::stringstream ss;
    ss << "";

    if (gq.getProductType() != GameQuery::PT_EE || !gq.isSupportedVersion()) {
        ss << "Playing";
    }
    else {

        if (!gq.isLoaded()) {
            ss << "Loading...";
            return ss.str();
        }

        switch (gq.getCurrentScreen())
        {
            case GameQuery::ST_Menu:
                ss << "In Menu";
                break;
            case GameQuery::ST_Lobby:
                ss << "In Lobby";
                break;
            case GameQuery::ST_PlayingSolo:
                ss << "Singleplayer";
                break;
            case GameQuery::ST_PlayingOnline:
                ss << "Multiplayer";
                break;
            case GameQuery::ST_Unknown:
            default:
                break;
        }
    }

    if (gq.isMinimized() && gq.isNeoEE())
        ss << " (NeoEE Minimized)";
    else if (gq.isNeoEE())
        ss << " (NeoEE)";
    else if (gq.isMinimized())
        ss << " (Minimized)";

    return ss.str();
}

void EEDiscord::updatePresenceThread()
{
    Logger::showMessage("Enter Presence Thread...", "EEDiscord");

    auto lastActive = std::chrono::high_resolution_clock::now();

    while (!_stop) {
        _mtx.lock();

        if (_gq->getProductType() == GameQuery::PT_EE) {
            _activity.GetAssets().SetLargeImage("logo");
            _activity.GetAssets().SetLargeText("Empire Earth");
        }
        else if (_gq->getProductType() == GameQuery::PT_AoC)
        {
            _activity.GetAssets().SetLargeImage("logo");
            _activity.GetAssets().SetLargeText("Empire Earth: The Art of Conquest");
        }

        _activity.SetDetails(buildDetailString(*_gq).c_str());
        _activity.SetState(buildStateString(*_gq).c_str());

        // AFK Management
        if (_gq->isMinimized()) {
            auto current = std::chrono::high_resolution_clock::now();
            auto minElapsed = std::chrono::duration_cast<std::chrono::minutes>(current - lastActive);
            if (minElapsed.count() >= 5) {
                _activity.GetAssets().SetSmallImage("afk");
                _activity.GetAssets().SetSmallText("Away From Keyboard");
            }
        }
        else {
            lastActive = std::chrono::high_resolution_clock::now();
            _activity.GetAssets().SetSmallImage("");
            _activity.GetAssets().SetSmallText("");
        }

        Logger::showMessage("Updating Presence", "EEDiscord");
        _state.core->ActivityManager().UpdateActivity(_activity, [](discord::Result result) {
            if (result == discord::Result::Ok)
                Logger::showMessage("Succeeded updating activity", "EEDiscord");
            else
                Logger::showMessage("Failed updating activity", "EEDiscord", true);
        });
        _mtx.unlock();
        Sleep(5000);
    }
}

void EEDiscord::updateCallbackThread()
{
    Logger::showMessage("Enter Callback Thread...", "EEDiscord");
    while (!_stop) {
        _mtx.lock();
        _state.core->RunCallbacks();
        _mtx.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

void EEDiscord::onStart()
{
    printCredit();

    if (!doesFileExist(L"discord_game_sdk.dll")) {
        Logger::showMessage("discord_game_sdk.dll missing!", "EEDiscord", true);
        return;
    }

    while (!IsProcessRunning(L"Discord.exe") || !IsProcessRunning(L"discord.exe")) {
        Logger::showMessage("Discord not running...", "EEDiscord");
        Sleep(10000);
    }

    discord::Result result;
    if (_gq->getProductType() == GameQuery::PT_EE)
        result = discord::Core::Create(782679873856077914, DiscordCreateFlags_Default, &_core);
    else if (_gq->getProductType() == GameQuery::PT_AoC)
        result = discord::Core::Create(783353615670706217, DiscordCreateFlags_Default, &_core);
    else {
        Logger::showMessage("Unable to identify game product! Did you renamed the executable? Exiting...", "EEDiscord", true);
        return;
    }

    _state.core.reset(_core);
    if (!_state.core) {
        std::stringstream ss;
        ss << "Failed to instantiate discord core! (err " << static_cast<int>(result) << ")";
        Logger::showMessage(ss.str(), "EEDiscord", true);
        return;
    }

    _activity.SetType(discord::ActivityType::Playing);
    _activity.GetTimestamps().SetStart(std::time(nullptr));

    auto updatePresenceThreadLamda = [](void* data) -> unsigned int
    {
        EEDiscord* context = static_cast<EEDiscord*>(data);
        context->updatePresenceThread();
        return 0;
    };

    auto updateCallbackThreadLamda = [](void* data) -> unsigned int
    {
        EEDiscord* context = static_cast<EEDiscord*>(data);
        context->updateCallbackThread();
        return 0;
    };

    HANDLE initPresenceThreadHandle = (HANDLE)_beginthreadex(0, 0, updatePresenceThreadLamda, this, 0, 0);
    HANDLE initCallbackThreadHandle = (HANDLE)_beginthreadex(0, 0, updateCallbackThreadLamda, this, 0, 0);
    Logger::showMessage("Exiting thread...", "EEDiscord");
}

void EEDiscord::onStop()
{
    _mtx.lock();
    _stop = true;
    _mtx.unlock();
}