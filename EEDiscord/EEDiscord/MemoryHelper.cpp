#include "pch.h"

#include "MemoryHelper.h"

/* fiddle around with the pointers */
HMODULE getBaseAddress() {
    return GetModuleHandle(NULL);
}

DWORD* calcAddress(DWORD appl_addr) {
    return (DWORD*)((DWORD)getBaseAddress() + appl_addr);
}

DWORD* tracePointer(memoryPTR* patch) {
    DWORD* location = calcAddress(patch->base_address);

    for (int n : patch->offsets) {
        location = (DWORD*)(*location + n);
    }

    return location;
}
