#include "ScopedSpinLock.hpp"

ScopedSpinLock::~ScopedSpinLock()
{
	// https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-kereleasespinlock
	KeReleaseSpinLock(&m_spinLock, m_oldIRQL);
}

ScopedSpinLock::ScopedSpinLock(KSPIN_LOCK& spinLock) :m_spinLock(spinLock)
{
	// https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-keacquirespinlock
	m_oldIRQL = KeAcquireSpinLockRaiseToDpc(&m_spinLock);
}