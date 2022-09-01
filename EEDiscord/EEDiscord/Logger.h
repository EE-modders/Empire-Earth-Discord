#pragma once

#include "Utils.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <mutex>

class Logger
{

private:
#ifdef _DEBUG
    std::wstring _dllName;
#endif // !_DEBUG
    std::ofstream _ofs;
    std::mutex _mtx;
    FILE* _f = NULL;

public:
    Logger() = default;
    ~Logger() = default;

    static Logger& getInstance() {
        static Logger logger;
        return logger;
    }

    static void init(std::wstring dllName)
    {
        std::lock_guard<std::mutex> lck(getInstance()._mtx);

        /* We don't really need logging for that DLL, let's disable it
        std::wstring logName = dllName + L".log";
        LPCWSTR logNameC = logName.c_str();
        
        if (doesFileExist(logNameC) && fileSize(logNameC) > 2 Mo * 100 * 100 * 100)
            DeleteFile(logNameC);

        if (!getInstance()._ofs.is_open())
            getInstance()._ofs.open(logName, std::ios::out | std::ios::app);
        */

#ifdef _DEBUG
        if (!GetConsoleWindow())
            AllocConsole();
        freopen_s(&(getInstance()._f), "CONOUT$", "w", stdout);
        getInstance()._dllName = dllName;
#endif // !_DEBUG

    }

    static void showMessage(std::string msg, std::string scope = "", bool error = false, bool show_time = true)
    {
        std::lock_guard<std::mutex> lck(getInstance()._mtx);

        /*
        if (!getInstance()._ofs.is_open())
            return;
        */

        std::stringstream ss;

        if (show_time)
            ss << "[" << currentDateTime("%H:%M:%S %d/%m/%Y") << "] ";

        if (error)
            ss << "[ERR] ";
        else
            ss << "[INF] ";

        if (!scope.empty())
            ss << "(" << scope << ") ";
        ss << msg;

#ifdef _DEBUG
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), error ? 4 : 8);
        std::wcout << "<" << getInstance()._dllName << "> ";
        std::cout << ss.str() << std::endl;
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
#endif // !_DEBUG
        getInstance()._ofs << ss.str() << std::endl;

        getInstance()._ofs.flush();
    }

};