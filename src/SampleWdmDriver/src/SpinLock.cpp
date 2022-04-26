#include "SpinLock.hpp"

SpinLock::~SpinLock()
{

}

SpinLock::SpinLock()
{
	// https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-keinitializespinlock
	KeInitializeSpinLock(&SpinLockHandle);
}
