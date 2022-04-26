#pragma once
#include <Ntddk.h>

/* 
* Note the following limitations from MSDN: https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-keacquirespinlock
	* The code within a critical region guarded by an spin lock must neither be pageable nor make any references to pageable data.
	* The code within a critical region guarded by a spin lock can neither call any external function that might access pageable data or raise an exception, nor can it generate any exceptions.
	* The caller should release the spin lock with KeReleaseSpinLock as quickly as possible.
*/
class ScopedSpinLock
{
	public:
		virtual ~ScopedSpinLock();
		ScopedSpinLock(KSPIN_LOCK& spinLock);

	protected:
		KSPIN_LOCK& m_spinLock;
		KIRQL m_oldIRQL;
};