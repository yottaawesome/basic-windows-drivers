#include <ntddk.h>

// As per https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-exallocatepoolwithtag
// Specify the pool tag as a non-zero character literal of one to to four characters delimited by single 
// quotation marks (for example, 'Tag1'). The string is usually specified in reverse order (for example, '1gaT').
// This is because of little endianness.
#define DRIVER_TAG 'dcba'

UNICODE_STRING g_RegistryPath{};

extern "C" NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath);
extern "C" void DriverUnload(_In_ PDRIVER_OBJECT DriverObject);

//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sample driver: DriverEntry() called\n");
NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sample driver: DriverEntry() called\n"));

	DriverObject->DriverUnload = DriverUnload;

	g_RegistryPath.Buffer = (wchar_t*)ExAllocatePoolWithTag(
		PagedPool,
		RegistryPath->Length,
		DRIVER_TAG
	);
	if (g_RegistryPath.Buffer == nullptr)
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

	ExFreePool(g_RegistryPath.Buffer); // Equivalent to: ExFreePoolWithTag(g_RegistryPath.Buffer, 0);
}
