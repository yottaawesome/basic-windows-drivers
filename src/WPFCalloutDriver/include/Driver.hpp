#include <ntddk.h>
#include <wdf.h>
#include <initguid.h>
#include <Wdmsec.h>

extern "C" NTSTATUS DriverEntry(
	_In_ PDRIVER_OBJECT  DriverObject,
	_In_ PUNICODE_STRING RegistryPath
);

void DriverUnload(_In_ WDFDRIVER DriverObject);
