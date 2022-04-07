#pragma once
#include "Headers.hpp"

namespace ToyDriver::Globals
{
	static WDFDEVICE WDFDriverDevice = { 0 };
	static PDEVICE_OBJECT DriverDeviceObject = nullptr;
}
