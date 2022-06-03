#include <ntddk.h>
#include "Operators.hpp"
#include "SpinLock.hpp"
#include "ScopedSpinLock.hpp"

UNICODE_STRING g_RegistryPath{};

extern "C" NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath);
extern "C" void DriverUnload(_In_ PDRIVER_OBJECT DriverObject);

NTSTATUS AllocateMemory(_In_ PUNICODE_STRING RegistryPath)
{
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

struct BranchIntegerEntry
{
	LIST_ENTRY Entry;
	int BranchValue;
};

struct TrunkIntegerEntry
{
	LIST_ENTRY Entry;
	LIST_ENTRY BranchRoot;
	int TrunkValue;
};

// https://docs.microsoft.com/en-us/windows-hardware/drivers/kernel/singly-and-doubly-linked-lists
class IntList
{
	public:
		~IntList()
		{
			Destroy();
		}

		void Destroy()
		{
			if (IsListEmpty(&m_integerListTrunkRoot))
				return;
			LIST_ENTRY* currentEntry = &m_integerListTrunkRoot;
			while (true)
			{
				LIST_ENTRY* entry = ExInterlockedRemoveHeadList(currentEntry, &m_spinLock.SpinLockHandle);
				if (entry == &m_integerListTrunkRoot)
					break;
				TrunkIntegerEntry* trunkEntry = CONTAINING_RECORD(entry, TrunkIntegerEntry, Entry);
				LIST_ENTRY* removedBranchEntry = ExInterlockedRemoveHeadList(&trunkEntry->BranchRoot, &m_spinLock.SpinLockHandle);
				while (removedBranchEntry != &trunkEntry->BranchRoot)
				{
					delete removedBranchEntry;
					removedBranchEntry = ExInterlockedRemoveHeadList(&trunkEntry->BranchRoot, &m_spinLock.SpinLockHandle);
				}
				delete trunkEntry;
			}
		}

		NTSTATUS AddTrunkEntry(const int value)
		{
			TrunkIntegerEntry* newEntry = new(NonPagedPool) TrunkIntegerEntry{ .TrunkValue = value };
			if (!newEntry)
			{
				return STATUS_INSUFFICIENT_RESOURCES;
			}
			InitializeListHead(&newEntry->BranchRoot);
			ExInterlockedInsertTailList(&m_integerListTrunkRoot, &newEntry->Entry, &m_spinLock.SpinLockHandle);
		}

		NTSTATUS AddBranchEntry(const int rootValueToFind, const int branchValueToAdd)
		{
			if (IsListEmpty(&m_integerListTrunkRoot))
				return STATUS_NOT_FOUND;

			TrunkIntegerEntry* trunkEntry = CONTAINING_RECORD(
				m_integerListTrunkRoot.Flink,
				TrunkIntegerEntry,
				Entry
			);
			while (true)
			{
				if (trunkEntry->TrunkValue == rootValueToFind)
					return STATUS_SUCCESS;

				if (trunkEntry->Entry.Flink == &m_integerListTrunkRoot)
				{
					BranchIntegerEntry* cEntry = new(NonPagedPool) BranchIntegerEntry{ .BranchValue = branchValueToAdd };
					if (!cEntry)
						return STATUS_INSUFFICIENT_RESOURCES;
					ExInterlockedInsertTailList(&trunkEntry->BranchRoot, &cEntry->Entry, &m_spinLock.SpinLockHandle);
					return STATUS_SUCCESS;
				}
				trunkEntry = CONTAINING_RECORD(trunkEntry->Entry.Flink, TrunkIntegerEntry, Entry);
			}
			return STATUS_NOT_FOUND;
		}

		NTSTATUS DeleteTrunkEntry(const int value)
		{
			if (IsListEmpty(&m_integerListTrunkRoot))
			{
				return STATUS_NOT_FOUND;
			}
			TrunkIntegerEntry* trunkEntry = CONTAINING_RECORD(
				m_integerListTrunkRoot.Flink,
				TrunkIntegerEntry,
				Entry
			);
			while (true)
			{
				if (trunkEntry->TrunkValue == value)
				{
					// delete sublist entries
					LIST_ENTRY branchRootEntry = trunkEntry->BranchRoot;
					LIST_ENTRY* removedEntry = ExInterlockedRemoveHeadList(&trunkEntry->BranchRoot, &m_spinLock.SpinLockHandle);// RemoveTailList(&trunkEntry->BranchRoot);
					while (&branchRootEntry != removedEntry)
					{
						BranchIntegerEntry* cEntry = CONTAINING_RECORD(removedEntry, BranchIntegerEntry, Entry);
						delete cEntry;
						removedEntry = ExInterlockedRemoveHeadList(&trunkEntry->BranchRoot, &m_spinLock.SpinLockHandle);
					}
					// remove and destroy the top list
					// No interlocked variant of RemoveEntryList
					ScopedSpinLock s(m_spinLock.SpinLockHandle);
					RemoveEntryList(&trunkEntry->Entry);
					delete trunkEntry;
					return STATUS_SUCCESS;
				}

				if (trunkEntry->Entry.Flink == &m_integerListTrunkRoot)
					break;
				trunkEntry = CONTAINING_RECORD(trunkEntry->Entry.Flink, TrunkIntegerEntry, Entry);
			}

			return STATUS_NOT_FOUND;
		}

	private:
		LIST_ENTRY m_integerListTrunkRoot;
		SpinLock m_spinLock;
};

IntList* g_list;
//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sample driver: DriverEntry() called\n");
NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sample driver: DriverEntry() called\n"));

	DriverObject->DriverUnload = DriverUnload;
	
	g_list = new(NonPagedPool) IntList();
	
	delete g_list;

	return STATUS_SUCCESS;
}

void DriverUnload(_In_ PDRIVER_OBJECT DriverObject)
{
	UNREFERENCED_PARAMETER(DriverObject);
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sample driver: DriverUnload() called\n"));

	delete g_RegistryPath.Buffer;
	//ExFreePoolWithTag(g_RegistryPath.Buffer, 0);
}
