/*++

Module Name:

    public.h

Abstract:

    This module contains the common declarations shared by driver
    and user applications.

Environment:

    user and kernel

--*/

//
// Define an Interface Guid so that apps can find the device and talk to it.
//

DEFINE_GUID (GUID_DEVINTERFACE_WPFCalloutDriver,
    0xe8ffe7f1,0x8547,0x4090,0x97,0x71,0x16,0xd6,0x01,0x71,0xa8,0xa0);
// {e8ffe7f1-8547-4090-9771-16d60171a8a0}
