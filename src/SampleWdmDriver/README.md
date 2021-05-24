# Sample WDM Driver

## Introduction

This is based on the very first sample from Pavel Yosifovich's [Windows Kernel Programming](https://leanpub.com/windowskernelprogramming) book.

## Installation

1. Copy the built sys file to the target machine (make sure it has been [provisioned for driver development and deployment](https://docs.microsoft.com/en-us/windows-hardware/drivers/gettingstarted/provision-a-target-computer-wdk-8-1)).
2. Open an elevated command prompt and then navigate to where you copied the executable.
3. Install the driver: `sc create sample type= kernel binPath= <full path to WDM executable>`. You can verify the driver was installed by looking at `HKLM\System\CurrentControlSet\Services\sample` in the Registry.
4. Start the driver: `sc start sample`.
5. Verify the driver is running with a tool like Process Explorer.

Once you are satisfied, you can stop the sample driver with `sc stop sample` and delete it with `sc delete sample`.
