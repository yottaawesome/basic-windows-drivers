#pragma once
#include <stddef.h>
#include <ntddk.h>

// Needs to be declared as it's an overload
// See http://stackoverflow.com/questions/583003/overloading-new-delete
void* operator new(size_t bytesToAlloc, const POOL_TYPE poolType) noexcept(false);
