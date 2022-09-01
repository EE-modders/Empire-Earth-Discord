#include "pch.h"
#include "EEDiscord.h"

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

void EEDiscord::updatePresenceThread()
{
    Logger::showMessage("Enter Presence Thread...", "EEDiscord");

    auto lastActive = std::chrono::high_resolution_clock::now();

    while (true) {
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

        if (_gq->getProductType() == GameQuery::PT_EE) { // Yeah I only have EE :|
            std::stringstream ss;
            switch (_gq->getCurrentScreen())
            {
                case GameQuery::ST_Menu:
                    _activity.SetState("In Menu");
                    break;
                case GameQuery::ST_Lobby:
                    _activity.SetState("In Lobby");
                    break;
                case GameQuery::ST_PlayingSolo:
                    _activity.SetState("Playing Solo");
                    ss << "As \"" << _gq->getUsername() << "\"";
                    _activity.SetDetails(ss.str().c_str());
                    break;
                case GameQuery::ST_PlayingOnline:
                    _activity.SetState("Playing Online");
                    ss << "As \"" << _gq->getUsername() << "\"";
                    _activity.SetDetails(ss.str().c_str());
                    break;
                case GameQuery::ST_Unknown:
                default:
                    _activity.SetState("");
                    break;
            }
        }

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
    while (true) {
        _mtx.lock();
        _state.core->RunCallbacks();
        _mtx.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

void EEDiscord::onStart()
{
    printCredit();

    discord::Result result;
    if (_gq->getProductType() == GameQuery::PT_EE)
        result = discord::Core::Create(782679873856077914, DiscordCreateFlags_NoRequireDiscord, &_core);
    else if (_gq->getProductType() == GameQuery::PT_AoC)
        result = discord::Core::Create(783353615670706217, DiscordCreateFlags_NoRequireDiscord, &_core);
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
}