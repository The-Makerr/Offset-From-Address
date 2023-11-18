#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <string>
#include <sstream>

uintptr_t GetProcessBaseAddress(const std::wstring& processName) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to create snapshot." << std::endl;
        return 0;
    }

    PROCESSENTRY32 processEntry;
    processEntry.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(snapshot, &processEntry)) {
        do {
            if (processName == processEntry.szExeFile) {
                HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processEntry.th32ProcessID);
                if (processHandle != NULL) {
                    HMODULE hMods[1024];
                    DWORD cbNeeded;
                    if (EnumProcessModules(processHandle, hMods, sizeof(hMods), &cbNeeded)) {
                        uintptr_t baseAddress = reinterpret_cast<uintptr_t>(hMods[0]);
                        CloseHandle(processHandle);
                        CloseHandle(snapshot);
                        return baseAddress;
                    }
                }
            }
        } while (Process32Next(snapshot, &processEntry));
    }

    CloseHandle(snapshot);
    return 0; 
}

int main() {

    std::wstring processName;
    std::wcout << L"Enter the process name: ";
    std::getline(std::wcin, processName);
    while (true) {
        std::wstring targetAddressStr;
        std::wcout << L"Enter the target address (in hexadecimal format): ";
        std::getline(std::wcin, targetAddressStr);

        std::wstringstream converter;
        converter << std::hex << targetAddressStr;
        uintptr_t targetAddress;
        converter >> targetAddress;

        uintptr_t baseAddress = GetProcessBaseAddress(processName);
        if (baseAddress != 0) {
            uintptr_t offset = targetAddress - baseAddress;
            std::wcout << L"Base address of " << processName << L": 0x" << std::hex << baseAddress << std::endl;
            std::wcout << L"Offset from base address to target address: 0x" << offset << std::endl;
        }
        else {
            std::wcout << L"Process \"" << processName << L"\" not found or unable to retrieve base address." << std::endl;
            return 0;
        }
    }
    return 0;
}
