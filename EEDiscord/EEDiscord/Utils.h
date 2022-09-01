#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <mutex>
#include <codecvt>
#include <algorithm>

static std::wstring getDllPath()
{
    TCHAR dllPath[MAX_PATH];
    HMODULE hModule;
    // Passing a static function to recover the DLL Module Handle
    if (!GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPWSTR)&getDllPath, &hModule)) {
        throw std::exception("Unable to get module handle of an internal function");
    }
    else {
        GetModuleFileName(hModule, dllPath, MAX_PATH);
    }
    return std::wstring(dllPath);
}

static std::wstring getFileName(const std::wstring& path, bool withExtention)
{
    std::wstring result = path.substr(path.find_last_of(L"/\\") + 1);
    if (!withExtention)
        result = result.substr(0, result.find_last_of('.'));
    return result;
}

static LONGLONG fileSize(const wchar_t* name)
{
    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (!GetFileAttributesEx(name, GetFileExInfoStandard, &fad))
        return -1; // error condition, could call GetLastError to find out more
    LARGE_INTEGER size;
    size.HighPart = fad.nFileSizeHigh;
    size.LowPart = fad.nFileSizeLow;
    return size.QuadPart;
}

static void ToUpper(std::wstring& wstr)
{
    std::transform(wstr.begin(), wstr.end(), wstr.begin(), ::toupper);
}

static void ToUpper(std::string& str)
{
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
}

static bool doesFileExist(LPCWSTR lpszFilename)
{
    DWORD attr = GetFileAttributes(lpszFilename);
    return ((attr != INVALID_FILE_ATTRIBUTES) && !(attr & FILE_ATTRIBUTE_DIRECTORY));
}

static bool doesFolderExist(LPCWSTR lpszFoldername)
{
    DWORD attr = GetFileAttributes(lpszFoldername);
    return ((attr != INVALID_FILE_ATTRIBUTES) && (attr & FILE_ATTRIBUTE_DIRECTORY));
}

static const std::string currentDateTime(std::string format) {
    time_t now = time(0);
    struct tm tstruct;
    char buf[256];
    localtime_s(&tstruct, &now);
    strftime(buf, sizeof(buf), format.c_str(), &tstruct);
    return buf;
}

static std::wstring utf8ToUtf16(const std::string& utf8Str)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
    return conv.from_bytes(utf8Str);
}

static std::string utf16ToUtf8(const std::wstring& utf16Str)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
    return conv.to_bytes(utf16Str);
}