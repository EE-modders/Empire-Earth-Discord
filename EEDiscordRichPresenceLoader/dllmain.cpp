// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <metahost.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <shlwapi.h>
#pragma comment(lib, "mscoree.lib")

#include "MonaLisas_CPP_Discord.h"

extern void LaunchDll(
    unsigned char* dll, size_t dllLength,
    char const* className, char const* methodName);

void LaunchDll(
    unsigned char* dll, size_t dllLength,
    char const* className, char const* methodName)
{}

BOOL APIENTRY DllMain(HMODULE h,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        DWORD Current_Game_ProcessID = GetCurrentProcessId();
        if (Current_Game_ProcessID > 0) {
            std::wstring MutexString = L"MonaCheck_" + std::to_wstring(Current_Game_ProcessID);
            HANDLE handleMutex = CreateMutexW(NULL, TRUE, MutexString.c_str());
            bool LoadDllDuringThisCall = true;
           // MessageBoxW(NULL, L"DLL LOADING", L"Fuck", MB_OK | MB_ICONINFORMATION);
            if (GetLastError() == ERROR_ALREADY_EXISTS)
            {
                LoadDllDuringThisCall = false;
                // Already Loaded
                //MessageBoxW(NULL, L"THIS CALL WILL BE SKIPPED OK?", L"MUTEX ALREADY PRESENT OMG!", MB_OK | MB_ICONINFORMATION);
            }
            if (LoadDllDuringThisCall) {
                //New: Discord Thread:
                Start_Mona_Lisa_Discord_Thread();
                break;

                //Unused, but keep the code in case we need to call external DLL function in the future.

                /*wchar_t CurrentWorkingDirectoryWhereExeIs[MAX_PATH];
                std::wstring ress = std::wstring(CurrentWorkingDirectoryWhereExeIs, GetModuleFileNameW(NULL, CurrentWorkingDirectoryWhereExeIs, MAX_PATH));
                PathRemoveFileSpecW(CurrentWorkingDirectoryWhereExeIs);
                std::wstring CurrentExeWorksPath = CurrentWorkingDirectoryWhereExeIs;

                std::wstring EE_DLL = CurrentExeWorksPath + L"\\EEDiscordRichPresence.dll";

                typedef void (WINAPI* MonaFuncCall)();
                MonaFuncCall MonaNudesLeak;

                HMODULE EE2DiscordRichPresenceDLL = LoadLibraryW(EE_DLL.c_str());
                if (EE2DiscordRichPresenceDLL != NULL) {
                    MonaNudesLeak = (MonaFuncCall)GetProcAddress(EE2DiscordRichPresenceDLL, "Start");
                    if (MonaNudesLeak) {
                        //MessageBoxA(NULL, "LOADED. NEXT MESSAGE BOX FROM C# SHOULD APPEAR AFTER CLICKING OK", "DLL", MB_OK | MB_ICONINFORMATION);
                        MonaNudesLeak();
                    } else {
                        MessageBoxA(NULL, "FAILED TO LOAD: STARTING FUNCTION NOT FOUND!", "DLL", MB_OK | MB_ICONINFORMATION);
                    }
                }
                else {
                    std::wstring ErrorCode = std::to_wstring(GetLastError());
                    MessageBoxW(NULL, ErrorCode.c_str(), L"EEDiscordRichPresence.dll Not found? Error code:", MB_OK | MB_ICONINFORMATION);
                }*/
            }
        }
        else {
           // MessageBoxW(NULL, L"Unable to get EE process ID!", L"CRITICAL ERROR!!!!", MB_OK | MB_ICONINFORMATION);
        }
        break;
    }
    case DLL_THREAD_ATTACH:
    {
        break;
    }
    case DLL_THREAD_DETACH:
    {
        break;
    }
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}