#include "Util.hpp"

namespace ToyDriver::Util
{
    DWORD GetProcessId(const FWPS_FILTER3* filter)
    {
        if (!filter)
            return 0;
        if (!filter->providerContext)
            return 0;
        if (!filter->providerContext->dataBuffer)
            return 0;
        if (!filter->providerContext->dataBuffer->size)
            return 0;

        if (filter->providerContext->dataBuffer->size != sizeof(DWORD))
        {
            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "GetProcessId(): data size mismatch\n"));
            return 0;
        }

        return *reinterpret_cast<DWORD*>(filter->providerContext->dataBuffer->data);
    }

    void LogFilter(const FWPS_FILTER3* filter)
    {
        if (!filter)
        {
            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "filter is null\n"));
            return;
        }
        if (!filter->providerContext)
        {
            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "providerContext is null\n"));
            return;
        }
        if (!filter->providerContext->dataBuffer)
        {
            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "dataBuffer is null\n"));
            return;
        }
        if (!filter->providerContext->dataBuffer->size)
        {
            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "dataBuffer is empty\n"));
            return;
        }

        // filter->providerContext->providerData is always null, not matter what I pass for the
        // providerData at the filter, callout, or provider object creation stages. The MSDN
        // docs don't really provide any useful information as to why, and the only tangible
        // sources online refer only to filter->providerContext->dataBuffer instead.
    }
}
