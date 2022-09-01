#pragma once

#include "EEDiscord.h"

#include <string>
#include <iostream>
#include <windef.h>


/*
    #################################
    #        EE DLL Template        #
    #################################

    YOU SHOULD NOT CHANGE THIS FILE
    EXCEPT IF YOU KNOW WHAT YOU ARE DOING!

*/


static const std::string TEMPLATE_VERSION_STR = "1.0.1";
static const unsigned int TEMPLATE_VERSION_MAJOR = 1;
static const unsigned int TEMPLATE_VERSION_MINOR = 0;
static const unsigned int TEMPLATE_VERSION_PATCH = 1;

class Library
{
private:
    EEDiscord* _project = nullptr;
    std::wstring _dllPath;

public:
    Library() {
        Logger::showMessage("Loading...", "Library");
        Logger::showMessage("Based on EE DLL Template " + TEMPLATE_VERSION_STR + "...", "Library");

        _dllPath = getDllPath();

        // Project
        _project = new EEDiscord();

        Logger::showMessage("Loaded!", "Library");
    }

    ~Library() {
        Logger::showMessage("Unloading...", "Library");
        delete _project;
        Logger::showMessage("Unloaded!", "Library");
    }

    void StartLibraryThread()
    {
        Logger::showMessage("Enter Thread!", "LibraryThread");
        _project->onStart();

        while (KEEP_ALIVE)
            Sleep(10000);
    }

    EEDiscord* getProject() {
        return _project;
    }

    std::wstring getDllPath()
    {
        return _dllPath;
    }
};