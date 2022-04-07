#pragma once
#include <guiddef.h>

namespace ToyDriver::Identifiers
{
    // {2CC08215-0458-4B8B-9B03-1A00F4914599}
    static const GUID WFP_OUTBOUND_IPV4_CALLOUT_GUID =
    { 0x2cc08215, 0x458, 0x4b8b, { 0x9b, 0x3, 0x1a, 0x0, 0xf4, 0x91, 0x45, 0x99 } };

    // {B0440EFA-87CC-4727-8D95-972FC02FA4CA}
    static const GUID WFP_INBOUND_IPV4_CALLOUT_GUID =
    { 0xb0440efa, 0x87cc, 0x4727, { 0x8d, 0x95, 0x97, 0x2f, 0xc0, 0x2f, 0xa4, 0xca } };

    // {56E4C747-B3F4-464F-89CC-511D1999B27B}
    static const GUID WFP_INBOUND_ICMP_ERROR_CALLOUT_GUID =
    { 0x56e4c747, 0xb3f4, 0x464f, { 0x89, 0xcc, 0x51, 0x1d, 0x19, 0x99, 0xb2, 0x7b } };

    // {06A7CEA1-7826-4330-B2F9-4D801BDC0B37}
    static const GUID WFP_OUTBOUND_TCP_CALLOUT_GUID =
    { 0x6a7cea1, 0x7826, 0x4330, { 0xb2, 0xf9, 0x4d, 0x80, 0x1b, 0xdc, 0xb, 0x37 } };
}