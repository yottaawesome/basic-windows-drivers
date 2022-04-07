#pragma once
#include "Headers.hpp"

namespace ToyDriver::Util
{
    DWORD GetProcessId(const FWPS_FILTER3* filter);
    void LogFilter(const FWPS_FILTER3* filter);
}
