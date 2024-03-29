module;

#include <Windows.h>
#include <guiddef.h>

export module wfpcontroller.identifiers;

export namespace WFPController::Identifiers
{
	// 2CC08215-0458-4B8B-9B03-1A00F4914599
	extern const GUID OutboundIPv4CalloutKey =
	{ 0x2CC08215, 0x0458, 0x4B8B, {0x9B, 0x03, 0x1A, 0x00, 0xF4, 0x91, 0x45, 0x99} };

	extern const GUID InboundICMPErrorCalloutKey =
	{ 0x56e4c747, 0xb3f4, 0x464f, { 0x89, 0xcc, 0x51, 0x1d, 0x19, 0x99, 0xb2, 0x7b } };

	extern const GUID OutboundICMPErrorCalloutKey =
	{ 0xeb79f29b, 0xccb, 0x48d7, { 0x8a, 0x33, 0x45, 0xcb, 0x31, 0x1e, 0x71, 0x25 } };

	// {B0440EFA-87CC-4727-8D95-972FC02FA4CA}
	extern const GUID InboundIPV4CalloutKey =
	{ 0xb0440efa, 0x87cc, 0x4727, { 0x8d, 0x95, 0x97, 0x2f, 0xc0, 0x2f, 0xa4, 0xca } };

	// {06A7CEA1-7826-4330-B2F9-4D801BDC0B37}
	extern const GUID OutboundTCPCalloutKey =
	{ 0x6a7cea1, 0x7826, 0x4330, { 0xb2, 0xf9, 0x4d, 0x80, 0x1b, 0xdc, 0xb, 0x37 } };

	// 270B7DD9-17E7-480E-B25F-04BB81F42495
	extern const GUID ProviderKey =
	{ 0x270B7DD9, 0x17E7, 0x480E, {0x52, 0x5F, 0x04, 0xBB, 0x81, 0xF4, 0x24, 0x95} };

	// {91A8A45B-9504-407E-A28C-FEA51F8D6C6D}
	extern const GUID SublayerKey =
	{ 0x91a8a45b, 0x9504, 0x407e, { 0xa2, 0x8c, 0xfe, 0xa5, 0x1f, 0x8d, 0x6c, 0x6d } };

	// {7CC93455-562A-4691-AEA2-621F34ED3784}
	extern const GUID ProviderContextKey =
	{ 0x7cc93455, 0x562a, 0x4691, { 0xae, 0xa2, 0x62, 0x1f, 0x34, 0xed, 0x37, 0x84 } };
}
