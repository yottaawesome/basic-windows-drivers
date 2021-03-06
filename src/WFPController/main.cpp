#include <iostream>
#include <memory>
#include <string>
#include <source_location>
#include <format>
#include <stdexcept>
#include <Windows.h>
#include <fwpmu.h>
#include <Winternl.h>
#include <guiddef.h>

import wfpcontroller.wfp;
import wfpcontroller.winsock.socket;
import wfpcontroller.wfp.wfperror;
import wfpcontroller.winsock.winsockinit;

// WFP error codes: https://docs.microsoft.com/en-us/windows/win32/fwp/wfp-error-codes
//struct FilterEngineDeleter
//{
//    void operator()(HANDLE engineHandle)
//    {
//        DWORD status = FwpmCalloutDeleteByKey0(engineHandle, &WFP_PROVIDER_GUID);
//        if (status != ERROR_SUCCESS)
//            std::cout << "FwpmCalloutDeleteByKey0() failed\n";
//    }
//};
//using FilterEngineUniquePtr = std::unique_ptr<std::remove_pointer<HANDLE>::type, FilterEngineDeleter>;

int main(int argc, char* argv[])
{
    HANDLE engineHandle = nullptr;
    try
    {
        WFPController::WFP::WFPEngine engine;
        WFPController::WinSock::WinSockInit init;

        bool execute = true;
        std::string input;
        while (execute)
        {
            std::cout << "Enter input...\n";
            std::cin >> input;

            if (input == "exit")
            {
                return 0;
            }
            else if (input == "open")
            {
                engine.OpenFilterEngine();
            }
            else if (input == "close")
            {
                engine.Close();
            }
            else if (input == "register")
            {
                engine.RegisterProvider();
            }
            else if (input == "context")
            {
                engine.AddContext();
            }
            else if (input == "callouts")
            {
                engine.AddCallouts();
            }
            else if (input == "sublayer")
            {
                engine.AddSublayer();
            }
            else if (input == "filter")
            {
                engine.AddFilters();
            }
            else if (input == "all")
            {
                engine.OpenFilterEngine();
                engine.RegisterProvider();
                engine.AddContext();
                engine.AddSublayer();
                engine.AddCallouts();
                engine.AddFilters();
            }
            else if (input == "connect")
            {
                WFPController::WinSock::Socket socket(L"142.250.70.164", 80);
                socket.Connect();
            }
            else if (input == "ttl")
            {
                try
                {
                    WFPController::WinSock::Socket socket(L"142.250.70.164", 80);
                    socket.SetPreconnectTTL(1);
                    socket.Connect();
                }
                catch (const std::exception& ex)
                {
                    std::cout << ex.what() << std::endl;
                }
            }
            else
            {
                std::cout << "Error: invalid input\n";
            }
        }

        std::cout << "Bye!\n";
        return 0;
    }
    catch (const std::exception& ex)
    {
        std::cout << std::format("Bailing as an exception occurred: {}\n", ex.what());
        return 1;
    }
}
