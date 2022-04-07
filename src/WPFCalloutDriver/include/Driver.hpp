#include "Headers.hpp"

namespace ToyDriver
{
    void DriverUnload(_In_ WDFDRIVER DriverObject);
    extern "C" NTSTATUS DriverEntry(
        _In_ PDRIVER_OBJECT  DriverObject,
        _In_ PUNICODE_STRING RegistryPath
    );
}
