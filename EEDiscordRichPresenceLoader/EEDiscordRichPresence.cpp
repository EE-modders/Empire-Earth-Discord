// EEDiscordRichPresenceLoader.cpp : Defines the exported functions for the DLL.
//

#include "pch.h"
#include "framework.h"
#include "EEDiscordRichPresence.h"


// This is an example of an exported variable
EE1DLLTEST_API int nEE1DLLTest=0;

// This is an example of an exported function.
EE1DLLTEST_API int fnEE1DLLTest(void)
{
    return 0;
}

// This is the constructor of a class that has been exported.
CEE1DLLTest::CEE1DLLTest()
{
    return;
}
