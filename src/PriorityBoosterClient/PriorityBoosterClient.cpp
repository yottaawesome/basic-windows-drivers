#include <iostream>
#include <format>
#include <Windows.h>
#include "../PriorityBooster/PriorityBoosterCommon.hpp"

int main(int argc, char* args[])
{
    if (argc < 3)
    {
        std::wcout 
            << "Usage: Booster <threadid> <priority>"
            << std::endl;
        return 0;
    }

    HANDLE hDevice = CreateFileW(
        L"\\\\.\\PriorityBooster",
        GENERIC_WRITE,
        FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr
    );
    if (hDevice == nullptr)
    {
        std::wcerr 
            << "CreateFileW() failed: "
            << GetLastError() 
            << std::endl;
        return 1;
    }

    ThreadData data{
        .ThreadId = std::stoul(args[1]),
        .Priority = std::stoi(args[2])
    };

    DWORD returned = 0;
    bool success = DeviceIoControl(
        hDevice,
        PriorityBooster::IoControl::IoctlPriorityBoosterSetPriority,
        &data, 
        sizeof(data), 
        nullptr, 
        0, 
        &returned, 
        nullptr
    );
  
    if (success == false)
    {
        std::wcerr
            << "DeviceIoControl() failed: "
            << GetLastError()
            << std::endl;
        return 1;
    }

    std::wcout << "Success!" << std::endl;

    return 0;
}

