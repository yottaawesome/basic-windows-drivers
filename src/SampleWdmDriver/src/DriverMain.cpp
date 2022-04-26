#include <ntddk.h>
#include "Operators.hpp"



UNICODE_STRING g_RegistryPath{};

extern "C" NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath);
extern "C" void DriverUnload(_In_ PDRIVER_OBJECT DriverObject);

//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sample driver: DriverEntry() called\n");
NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sample driver: DriverEntry() called\n"));

	DriverObject->DriverUnload = DriverUnload;

	g_RegistryPath.Buffer = new(PagedPool) wchar_t[RegistryPath->Length];
	if (!g_RegistryPath.Buffer)
	{
		KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, __FUNCSIG__ ": could not allocate memory for registry path \n"));
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, __FUNCSIG__ ": successfully allocated memory \n"));
	g_RegistryPath.MaximumLength = RegistryPath->Length;
	RtlCopyUnicodeString(&g_RegistryPath, (PCUNICODE_STRING)RegistryPath);

	return STATUS_SUCCESS;
}

void DriverUnload(_In_ PDRIVER_OBJECT DriverObject)
{
	UNREFERENCED_PARAMETER(DriverObject);
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sample driver: DriverUnload() called\n"));

	delete g_RegistryPath.Buffer;
	//ExFreePoolWithTag(g_RegistryPath.Buffer, 0);
}
