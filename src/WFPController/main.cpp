#include <iostream>
#include <memory>
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
    const DWORD status = FwpmEngineOpen0(
        nullptr,                   // The filter engine on the local system
        RPC_C_AUTHN_DEFAULT,    // Use the Windows authentication service
        nullptr,                   // Use the calling thread&#39;s credentials
        &session,               // There are session-specific parameters
        &engineHandle     // Pointer to a variable to receive the handle
    );
    if (status != ERROR_SUCCESS)
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
    const DWORD status = FwpmProviderAdd0(
        engineHandle,
        &provider,
        nullptr
    );
    if (status != ERROR_SUCCESS)
        throw std::runtime_error(std::format("Failed to add provider {}", status));
}

void AddCallouts(HANDLE engineHandle)
{
    unsigned calloutId;
    std::wstring calloutName = L"ToyDriverCallout";
    FWPM_CALLOUT0 mCallout = {
        .calloutKey = WFP_TEST_CALLOUT,
        .displayData = { .name = calloutName.data() },
        .providerKey = (GUID*)&WFP_PROVIDER_GUID,
        .applicableLayer = FWPM_LAYER_OUTBOUND_IPPACKET_V4,
    };
    const DWORD status = FwpmCalloutAdd0(
        engineHandle,
        &mCallout,
        nullptr,         // default security desc
        &calloutId
    );
    if (status != ERROR_SUCCESS)
        throw std::runtime_error(std::format("Failed to add callout {}", status));
}

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

void DeleteAllObjects(HANDLE engineHandle)
{
    DWORD status = FwpmCalloutDeleteByKey0(engineHandle, &WFP_TEST_CALLOUT);
    if (status != ERROR_SUCCESS)
        std::cout << std::format("FwpmCalloutDeleteByKey0() failed {:X}\n", status);

    // https://docs.microsoft.com/en-us/windows/win32/api/fwpmu/nf-fwpmu-fwpmproviderdeletebykey0
    status = FwpmProviderDeleteByKey0(engineHandle, &WFP_PROVIDER_GUID);
    if (status != ERROR_SUCCESS)
        std::cout << std::format("FwpmProviderDeleteByKey0() failed {:X}\n", status);

    // https://docs.microsoft.com/en-us/windows/win32/api/fwpmu/nf-fwpmu-fwpmengineclose0
    status = FwpmEngineClose0(engineHandle);
    if (status != ERROR_SUCCESS)
        std::cout << std::format("FwpmEngineClose0() failed {:X}\n", status);
}

int main(int argc, char* argv[])
{
    HANDLE engineHandle = nullptr;
    try
    {
        engineHandle = OpenFilterEngine();
        if (argc > 1)
        {
            DeleteAllObjects(engineHandle);
            return 0;
        }

        // Startup
        //FilterEngineUniquePtr enginePtr(engineHandle);
        RegisterProvider(engineHandle);
        AddCallouts(engineHandle);
        DeleteAllObjects(engineHandle);

        std::cout << "OK -- no errors." << std::endl;
        return 0;
    }
    catch (const std::exception& ex)
    {
        std::cout << ex.what() << std::endl;
        if (engineHandle)
            DeleteAllObjects(engineHandle);
    }
}

