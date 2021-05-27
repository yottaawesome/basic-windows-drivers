# Sample WDM Driver

## Introduction

This is based on the very first sample from Pavel Yosifovich's [Windows Kernel Programming](https://leanpub.com/windowskernelprogramming) book.

## Installation

1. Copy the built sys file to the target machine (make sure it has been [provisioned for driver development and deployment](https://docs.microsoft.com/en-us/windows-hardware/drivers/gettingstarted/provision-a-target-computer-wdk-8-1)).
2. Open an elevated command prompt and then navigate to where you copied the executable.
3. Install the driver: `sc create sample type= kernel binPath= <full path to WDM executable>`. You can verify the driver was installed by looking at `HKLM\System\CurrentControlSet\Services\sample` in the Registry.
4. Start the driver: `sc start sample`.
5. Verify the driver is running with a tool like Process Explorer.

## Removal

Once you are satisfied, you can stop the sample driver with `sc stop sample` and delete it with `sc delete sample`.

## What this driver does

The driver outputs some debug logging in its `DriverEntry()` and `DriverUnload()` routines. To view this output, you'll need to create the key `HKLM\SYSTEM\CurrentControlSet\Control\SessionManager\Debug Print Filter` and set a `DWORD` value `IHVDRIVER` to `0x8` for informational or `0xF` for all level output (note that this only takes effect on reboot). You can then view the debug output either through a remote Windows Debugger session or through something like SysinternalsSuite's dbgview utility (you'll need to run it elevated and set `Capture > Capture Kernel` to true).

## Additional notes

If you choose to alter the driver and rebuild it, you can simply replace the installed executable with the new executable on the target machine. There's no need to re-install the driver.
