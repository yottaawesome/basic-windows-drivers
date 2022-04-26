#pragma once
#include <Ntddk.h>

class ScopedSpinLock
{
	public:
		virtual ~ScopedSpinLock();
		ScopedSpinLock(KSPIN_LOCK& spinLock);

	protected:
		KSPIN_LOCK& m_spinLock;
		KIRQL m_oldIRQL;
};