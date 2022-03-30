#include <iostream>
#include <stdexcept>
#include <Windows.h>
#include <fwpmu.h>
#include <Winternl.h>

HANDLE OpenFilterEngine()
{
    HANDLE          engineHandle;       // handle for the open session to the filter engine
    FWPM_SESSION0   session = { 0 };
    // Open handle to the filtering engine
    NTSTATUS status = FwpmEngineOpen0(
        nullptr,                   // The filter engine on the local system
        RPC_C_AUTHN_DEFAULT,    // Use the Windows authentication service
        nullptr,                   // Use the calling thread&#39;s credentials
        &session,               // There are session-specific parameters
        &engineHandle     // Pointer to a variable to receive the handle
    );
    if (NT_ERROR(status))
    {
        throw std::exception("Failed to open filter engine handle");
    }
    return engineHandle;
}

int main(int argc, char* argv[])
{
    HANDLE handle = OpenFilterEngine();
    FwpmEngineClose0(handle);

    std::cout << "Hello World!\n";
    return 0;
}

