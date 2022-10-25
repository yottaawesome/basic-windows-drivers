# Basic Windows Drivers

## Introduction

Basic experimentation with Windows drivers [based off the MSDN docs](https://docs.microsoft.com/en-us/windows-hardware/drivers/gettingstarted/writing-a-very-small-kmdf--driver), Pavel Yosifovich's [Windows Kernel Programming book](https://leanpub.com/windowskernelprogramming) and [Developing Drivers with the Windows Driver Foundation](https://docs.microsoft.com/en-us/windows-hardware/drivers/wdf/developing-drivers-with-wdf) by Penny Orwick and Guy Smith.

## Debugging

You'll need to first configure a target machine or VM for deployment and debugging. [See here](https://docs.microsoft.com/en-us/windows-hardware/drivers/gettingstarted/provision-a-target-computer-wdk-8-1). Note that you need to ensure the target and host can ping each other. You may need to change the VM to use a Bridged connection and allow [File and Printer Sharing](https://stackoverflow.com/questions/18278409/cant-ping-a-local-vm-from-the-host) on both the host and the target. You may then need to set up your [Debug Print Filter](https://stackoverflow.com/questions/17109074/kdprintex-in-debugger-immediate-window-into-vs-2012-is-not-printing-any-msg) (important note: filter masks stored in the registry take effect during boot). Once deployed and installed on the target, you'll need to start a WndDbg session through an elevated command prompt e.g. `WinDbg -k net:port=<port>,key=<key>`. WinDbg is located in your Windows kit, e.g. `A:\Windows Kits\10\Debuggers\x64`.

## Building

You need Visual Studio 2022 and the [latest Windows Driver Kit (WDK) and accompanying SDK](https://docs.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk) to build and develop kernel components. If you receive obscure build errors related to inf2cat to dates in the `DriverVer` field in the `*.inf` files, ensure that in `Project Properties > Configuration Properties > Inf2Cat > General > Use local time` is set to `Yes (/uselocaltime)`. See [here](https://stackoverflow.com/questions/14148500/int2cat-driverver-set-to-incorrect-date) and [here](https://docs.microsoft.com/en-us/windows-hardware/drivers/devtest/stampinf-command-options) for additional information. You may need to rebuild more than if you receive some errors after doing a full solution clean. There may be some absolute paths (for my machine) referenced in the project files that you may need to adjust or remove for your local environment.

## Deploying

Provisioned VMs require binaries to be signed, or they'll be blocked by Microsoft Defender. As part of the VM provisioning process, development certificates for signing drivers will be generated, but you may still need to [generate self-signed test certificates yourself](https://docs.microsoft.com/en-us/windows-hardware/drivers/install/how-to-test-sign-a-driver-package). The `SignTool` comes with the Windows Kit (e.g. under `C:\Program Files (x86)\Windows Kits\10\App Certification Kit\`) and can be used like so to sign a binary: `SignTool.exe Sign /a /v /fd SHA256 <binary_path>` (note that older versions did not require the `fd` argument).

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
* Don't try to use C++ modules in kernel projects -- VS seemingly bugs out and locks the IFC files, causing compilation failures. [This is apparently a WDK issue](https://developercommunity.visualstudio.com/t/c-modules-in-kmdf-project/1560782). This happens regardless of setting the standard to C++20, and the only way to solve the issue is to manually kill the language server processes in Task Manager, which is a pain.
* For the Windows kernel, C++ is a fully-viable and effective language option for drivers. However, you don't have access to the STL and the C++ runtime is unavailable, which means no exception support, by default. That being said, you can find a kernel-ready implementation of the STL via [Johnny Shaw's stlkrn](https://github.com/jxy-s/stlkrn) and you can enable C++ exceptions using [Martin Vejnár's vcrtl](https://github.com/avakar/vcrtl). I've not used them personally, so I can't vouch for them.
* In the Windows registry, installed drivers are located under the `HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services` key.
* When drivers are installed, Windows will copy the driver's INF file and referenced files to the [driver store](https://learn.microsoft.com/en-us/windows-hardware/drivers/install/driver-store), which appears to be the `%SYSTEMROOT%\System32\DriverStore` directory. This behaviour was introduced in Windows Vista.
* If you stop a driver and then restart it (e.g. via the `net stop` and `net start` commands) and you get a "file not found" error, check to see that your driver cleans up all resources before exiting. Certain resources, such as callouts, not being cleaned up [can cause this issue](https://stackoverflow.com/a/69284447/7448661).
* When invoking any native Microsoft functions in the kernel, it's worthwhile checking the MSDN docs to ensure that you are running at the required IRQL to invoke those functions. This is because certain functions require the driver to be running at, above, or below a certain IRQL. Failure to do so will cause the function to fail or possibly bugcheck the system.

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
* [Developing Drivers with the Windows Driver Foundation](https://docs.microsoft.com/en-us/windows-hardware/drivers/wdf/developing-drivers-with-wdf) by Penny Orwick and Guy Smith. This is a useful book for writing Windows kernel drivers using the more modern WDF (as opposed to WDM) approach.
