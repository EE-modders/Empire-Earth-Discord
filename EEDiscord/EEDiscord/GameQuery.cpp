#include "pch.h"

#include "GameQuery.h"
#include "MemoryHelper.h"
#include "Utils.h"

GameQuery::GameQuery()
{
    TCHAR szExeFileName[MAX_PATH];
    GetModuleFileName(NULL, szExeFileName, MAX_PATH);
    std::wstring exeFileName = getFileName(std::wstring(szExeFileName), true);

    ToUpper(exeFileName);

    _productType = PT_Unknown;
    if (exeFileName.compare(L"EMPIRE EARTH.EXE") == 0)
        _productType = PT_EE;
    else if (exeFileName.compare(L"EE-AOC.EXE") == 0)
        _productType = PT_AoC;
};

bool GameQuery::isLoaded() {
    if (_productType == PT_EE)
        return *(int*)calcAddress(0x517BB8);
    return false;
}

bool GameQuery::isPlaying() {
    return 0 != *(int*)calcAddress(0x00518378 + 0x44);
}

bool GameQuery::inLobby() {
    return 0 != *(int*)calcAddress(0x00544254);
}

bool GameQuery::isMinimized()
{
    HWND hwnd = GetForegroundWindow();
    if (hwnd == NULL)
        return false;

    DWORD foregroundPid;
    if (GetWindowThreadProcessId(hwnd, &foregroundPid) == 0)
        return false;

    return (foregroundPid != GetCurrentProcessId());
    return false;
}

const char* GameQuery::getUsername()
{
    memoryPTR ptr = { 0x51930C, { 0x0 } };
    return (char*)tracePointer(&ptr);
}

GameQuery::ScreenType GameQuery::getCurrentScreen()
{
    bool playing = isPlaying();
    bool lobby = inLobby();

    if (playing && lobby)
        return ST_PlayingOnline;
    else if (playing && !lobby)
        return ST_PlayingSolo;
    else if (!playing && lobby)
        return ST_Lobby;
    else
        return ST_Menu;
}

const char* GameQuery::getGameBaseVersion()
{
    if (_productType == PT_EE)
        return (char*)calcAddress(0x04A9030);
    if (_productType == PT_AoC)
        return (char*)calcAddress(0x04BF570);
    return "";
}

const char* GameQuery::getGameDataVersion()
{
    memoryPTR memEEC{ 0x0513264, { 0x0 } };
    memoryPTR memAOC{ 0x0529A0C, { 0x0 } };

    if (_productType == PT_EE)
        return (char*)tracePointer(&memEEC);
    if (_productType == PT_AoC)
        return (char*)tracePointer(&memAOC);
    return "";
}

bool GameQuery::isSupportedVersion() {
    const char* supportedEEC = "2002.09.12.v2.00";
    const char* supportedAOC = "2002.8.17.v1.00";
    const char* current = getGameBaseVersion();

    if (_productType == PT_EE)
        return strcmp(supportedEEC, current) == 0;
    if (_productType == PT_AoC)
        return strcmp(supportedAOC, current) == 0;
    return false;
}

GameQuery::ProductType GameQuery::getProductType()
{
    return _productType;
}