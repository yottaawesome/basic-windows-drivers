#include <iostream>
#include <string>
#include <format>
#include <stdexcept>
#include <Windows.h>
#include <fwpmu.h>
#include <Winternl.h>
#include <guiddef.h>
#include "Guids.hpp"

HANDLE OpenFilterEngine()
{
    HANDLE          engineHandle;       // handle for the open session to the filter engine
    FWPM_SESSION0   session = { 0 };
    // Open handle to the filtering engine
    // https://docs.microsoft.com/en-us/windows/win32/api/fwpmu/nf-fwpmu-fwpmengineopen0
    const NTSTATUS status = FwpmEngineOpen0(
        nullptr,                   // The filter engine on the local system
        RPC_C_AUTHN_DEFAULT,    // Use the Windows authentication service
        nullptr,                   // Use the calling thread&#39;s credentials
        &session,               // There are session-specific parameters
        &engineHandle     // Pointer to a variable to receive the handle
    );
    if (NT_ERROR(status))
        throw std::runtime_error(std::format("Failed to open filter engine handle {}", status));
    return engineHandle;
}

void RegisterProvider(HANDLE engineHandle)
{
    if (!engineHandle)
        throw std::invalid_argument("engineHandle");

    std::wstring providerName = L"provider name";
    const FWPM_PROVIDER0  provider = {
        .providerKey = WFP_PROVIDER_GUID,
        .displayData = {.name = providerName.data()}
    };
    // https://docs.microsoft.com/en-us/windows/win32/api/fwpmu/nf-fwpmu-fwpmprovideradd0
    const NTSTATUS status = FwpmProviderAdd0(
        engineHandle,
        &provider,
        nullptr
    );
    if (NT_ERROR(status))
        throw std::runtime_error(std::format("Failed to add provider {}", status));
}

int main(int argc, char* argv[]) try
{
    // Startup
    HANDLE engineHandle = OpenFilterEngine();
    RegisterProvider(engineHandle);

    // Teardown
    // https://docs.microsoft.com/en-us/windows/win32/api/fwpmu/nf-fwpmu-fwpmproviderdeletebykey0
    DWORD status = FwpmProviderDeleteByKey0(engineHandle, &WFP_PROVIDER_GUID);
    if (status != ERROR_SUCCESS)
        throw std::runtime_error(std::format("Failed to delete provider {}", status));

    // https://docs.microsoft.com/en-us/windows/win32/api/fwpmu/nf-fwpmu-fwpmengineclose0
    status = FwpmEngineClose0(engineHandle);
    if (status != ERROR_SUCCESS)
        throw std::runtime_error(std::format("Failed to close the engine handle {}", status));

    std::cout << "OK -- no errors." << std::endl;
    return 0;
}
catch (const std::exception& ex)
{
    std::cout << ex.what() << std::endl;
}

