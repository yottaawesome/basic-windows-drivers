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

DEFINE_GUID (GUID_DEVINTERFACE_WDFTestDriver,
    0x048efc52,0x5ba1,0x4647,0x82,0x84,0xd6,0xe6,0xa6,0xa4,0x17,0x32);
// {048efc52-5ba1-4647-8284-d6e6a6a41732}
