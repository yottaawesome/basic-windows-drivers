#pragma once
#include <Ntddk.h>

class SpinLock
{
	public:
		virtual ~SpinLock();
		SpinLock();

		KSPIN_LOCK SpinLockHandle;
};