# Basic Windows Drivers

## Introduction

Basic experimentation with Windows drivers [based off the MSDN docs](https://docs.microsoft.com/en-us/windows-hardware/drivers/gettingstarted/writing-a-very-small-kmdf--driver) and Pavel Yosifovich's [Windows Kernel Programming book](https://leanpub.com/windowskernelprogramming).

## Debugging

You'll need to first configure a target machine or VM for deployment and debugging. [See here](https://docs.microsoft.com/en-us/windows-hardware/drivers/gettingstarted/provision-a-target-computer-wdk-8-1). Note that you need to ensure the target and host can ping each other. You may need to change the VM to use a Bridged connection and allow [File and Printer Sharing](https://stackoverflow.com/questions/18278409/cant-ping-a-local-vm-from-the-host) on both the host and the target. You may then need to set up your [Debug Print Filter](https://stackoverflow.com/questions/17109074/kdprintex-in-debugger-immediate-window-into-vs-2012-is-not-printing-any-msg) (important note: filter masks stored in the registry take effect during boot). Once deployed and installed on the target, you'll need to start WndDbg session through an elevated command prompt e.g. `WinDbg -k net:port=<port>,key=<key>`. WinDbg is located in your Windows kit, e.g. `A:\Windows Kits\10\Debuggers\x64`. Note that `KdPrintEx` does not appear to do anything in `DriverEntry()`, so don't expect output when walking through this symbol.

## Building

You need Visual Studio 2022 and the [latest Windows Driver Kit (WDK) and accompanying SDK](https://docs.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk) to build and develop kernel components. If you receive obscure build errors related to inf2cat to dates in the `DriverVer` field in the `*.inf` files, ensure that in `Project Properties > Configuration Properties > Inf2Cat > General > Use local time` is set to `Yes (/uselocaltime)`. See [here](https://stackoverflow.com/questions/14148500/int2cat-driverver-set-to-incorrect-date) and [here](https://docs.microsoft.com/en-us/windows-hardware/drivers/devtest/stampinf-command-options) for additional information. You may need to rebuild more than if you receive some errors after doing a full solution clean.

## Deploying

Provisioned VMs require binaries to be signed, or they'll be blocked by Microsoft Defender. As part of the VM provisioning process, development certificates for signing drivers will be generated. The `SignTool` comes with the Windows Kit (e.g. under `C:\Program Files (x86)\Windows Kits\10\App Certification Kit\`) and can be used like so to sign a binary: `SignTool.exe Sign -A -V <binary_path>`.

## WinDbg

* To add a breakpoint through WnDbg, use `bm <module>!<symbol>`, e.g. `bm KmdfHelloWorld!DriverEntry`
* To list all breakpoints, use `bl`;
* To list all running modules, use `lm`;
* To list all symbols in a module, use `x <module>!<symbol-search-string>` e.g. `x KmdfHelloWorld!Driver*`
* `bc` - clears a breakpoint from the list. Use `bc *` to clear all breakpoints.
* `bd` - disables a breakpoint. Use `bd *` to disable all breakpoints.
* `be` - enables a breakpoint. Use `be *` to enable all breakpoints.

See the [step-by-step lab](https://docs.microsoft.com/en-us/windows-hardware/drivers/debugger/debug-universal-drivers---step-by-step-lab--echo-kernel-mode-) for further information.

## Other notes and common problems

* If devcon.exe fails, a log is generated at `c:\windows\inf\setupapi.dev.log`.
* If you get an error about `No file digest algorithm specified. Please specify the digest algorithm with the /fd flag. Using /fd SHA256 is recommended and more secure than SHA1. Calling signtool with /fd sha1 is equivalent to the previous behavior. In order to select the hash algorithm used in the signing certificate's signature, use the /fd certHash option.`, right click the project > `Configuration Properties` > `Driver Signing` > `General` > `File Digest Algorithm` > `SHA256`.
* Newer versions of the WDK do not accept `Sample` as a class name in the INF file, and you'll also need to generate a new GUID.

## Additional resources

* [Download the WDK](https://docs.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk)
* [WinDbg basics](https://www.tenforums.com/tutorials/5558-windbg-basics-debugging-crash-dumps-windows-10-a.html)
* [Debug print message issues](https://social.msdn.microsoft.com/Forums/vstudio/en-US/4ec8c0fd-c399-4579-ac0b-d5d263820511/where-can-i-see-the-kdprintex-debug-message-in-vs-debugger-for-the-kmdfhelloworld-sample-project).
* [Reading and Filtering Debugging Messages](https://docs.microsoft.com/en-us/windows-hardware/drivers/debugger/reading-and-filtering-debugging-messages) (see also [this](https://stackoverflow.com/questions/17109074/kdprintex-in-debugger-immediate-window-into-vs-2012-is-not-printing-any-msg)).
* [Debug Windows Drivers - Step by Step Lab (Echo Kernel-Mode)](https://docs.microsoft.com/en-us/windows-hardware/drivers/debugger/debug-universal-drivers---step-by-step-lab--echo-kernel-mode-).
* [Getting Started with Windows Drivers](https://docs.microsoft.com/en-us/windows-hardware/drivers/develop/getting-started-with-windows-drivers).
* [Sample KMDF drivers](https://docs.microsoft.com/en-us/windows-hardware/drivers/wdf/sample-kmdf-drivers)
* [Windows Driver Samples](https://github.com/microsoft/Windows-driver-samples)
* [Debug View](https://docs.microsoft.com/en-us/sysinternals/downloads/debugview)
* [Getting Started with WinDbg (User-Mode)](https://docs.microsoft.com/en-us/windows-hardware/drivers/debugger/getting-started-with-windbg)
* [Pavel Yosifovich's tools](https://github.com/zodiacon/AllTools)
* [Sysinternals utilities](https://docs.microsoft.com/en-us/sysinternals/downloads/)
* [Managing hardware priorities](https://docs.microsoft.com/en-us/windows-hardware/drivers/kernel/managing-hardware-priorities)
