#pragma once

#include <vector>

struct memoryPTR {
    DWORD base_address;
    std::vector<int> offsets;
};

HMODULE getBaseAddress();

DWORD* calcAddress(DWORD appl_addr);
DWORD* tracePointer(memoryPTR* patch);