#pragma once

typedef unsigned long ULONG;

#define PRIORITY_BOOSTER_DEVICE 0x8000
/*
* We need to define out control codes using the CTL_CODE macro. A brief description of the meaning of these macro arguments:
* •DeviceType - identifies a type of device. This can be one of the FILE_DEVICE_xxx constants defined in the WDK headers,
* but this is mostly for hardware based drivers. For software drivers like ours, the number doesn’t matter much. Still,
* Microsoft’s documentation specifies that values for 3rd parties should start with 0x8000.
* •Function - an ascending number indicating a specific operation. If nothing else, this number must be different between
* different control codes for the same driver. Again, any number will do, but the official documentation says 3rd party
* drivers should start with 0x800.
* •Method - the most important part of the control code. It indicates how the input and
* output buffers provided by the client pass to the driver. We’ll deal with these values in detail in chapter 6. For
* our driver, we’ll use the simplest value METHOD_NEITHER. We’ll see its effect later in this chapter.
* •Access - indicates whether this operation is to the driver (FILE_WRITE_ACCESS), from the driver (FILE_READ_ACCESS), or
* both ways (FILE_ANY_ACCESS). Typical drivers just use FILE_ANY_ACCESS and deal with the actual request in the
* IRP_MJ_DEVICE_CONTROL handler.
*/
#define IOCTL_PRIORITY_BOOSTER_SET_PRIORITY CTL_CODE(PRIORITY_BOOSTER_DEVICE, 0x800, METHOD_NEITHER, FILE_ANY_ACCESS)
constexpr ULONG IoctlPriorityBoosterSetPriority = (ULONG)
	CTL_CODE(PRIORITY_BOOSTER_DEVICE, 0x800, METHOD_NEITHER, FILE_ANY_ACCESS);

struct ThreadData
{ 
	ULONG ThreadId; 
	int Priority; 
};
