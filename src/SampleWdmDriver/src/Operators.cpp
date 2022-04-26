#include <stddef.h>
#include <ntddk.h>

// As per https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-exallocatepoolwithtag
// Specify the pool tag as a non-zero character literal of one to to four characters delimited by single 
// quotation marks (for example, 'Tag1'). The string is usually specified in reverse order (for example, '1gaT').
// This is because of little endianness.
constexpr unsigned long DriverPoolTag = 'dcba';

void* operator new(size_t n, const POOL_TYPE poolType) noexcept(false)
{
    return ExAllocatePoolWithTag(poolType, n, DriverPoolTag);
}

void operator delete(void* p) noexcept(true)
{
    if (p)
        ExFreePoolWithTag(p, 0);
}

void operator delete(void* p, size_t) noexcept(true)
{
    if (p)
        ExFreePoolWithTag(p, 0);
}