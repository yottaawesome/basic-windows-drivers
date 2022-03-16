# WFP Callout Driver

## Introduction

This is a WIP bare-minimum WDF-based callout driver. It is primarily based on information and code stitched together from the following sources:

* [MSDN's docs on Callout Drivers](https://docs.microsoft.com/en-us/windows-hardware/drivers/network/roadmap-for-developing-wfp-callout-drivers). See in particular the sections under `Callout Driver Operations`.
* [Microsoft's WFP sample](https://docs.microsoft.com/en-us/samples/microsoft/windows-driver-samples/windows-filtering-platform-sample/)
* [teddysback's small callout driver](https://github.com/teddysback/netFilter)

## Installing

You must first [provision a VM](https://docs.microsoft.com/en-us/windows-hardware/drivers/gettingstarted/provision-a-target-computer-wdk-8-1) and meet all other pre-requisites, such as installing the WDK.
* Open the solution in VS2019 (VS2022 is not supported by the WDK as of the time of this writing) and configure the appropriate target.
* Right click on WFPCalloutDriver and select `Deploy`. This will deploy the driver to `%systemdrive%\drivertest\drivers` on the target machine.
* On the target machine, open an elevated command prompt and `cd` to `%systemdrive%\drivertest\drivers`.
* Run `driver-install.cmd` to install the driver. Run `driver-install.cmd -r` to uninstall the driver.

## Additional resources

* [Installing callout drivers (INF file guidance)](https://docs.microsoft.com/en-us/windows-hardware/drivers/network/inf-files-for-callout-drivers);
* [WFPSampler fails to build under Win10](https://docs.microsoft.com/en-us/answers/questions/233569/wfpsampler-fails-to-build-under-windows-10.html)
  * See also [this PR](https://github.com/microsoft/Windows-driver-samples/pull/538/files)
* [Differences between WDM and WDF drivers](https://docs.microsoft.com/en-us/windows-hardware/drivers/wdf/differences-between-wdm-and-kmdf)
* [Built-in callout identifiers](https://docs.microsoft.com/en-us/windows-hardware/drivers/network/built-in-callout-identifiers)
* [Windows Filtering Platform](https://docs.microsoft.com/en-us/windows/win32/fwp/windows-filtering-platform-start-page)
* [Using WDF to develop a driver](https://docs.microsoft.com/en-us/windows-hardware/drivers/wdf/using-the-framework-to-develop-a-driver)
* [Developing Drivers with the Windows Driver Foundation](https://docs.microsoft.com/en-us/windows-hardware/drivers/wdf/developing-drivers-with-wdf)
