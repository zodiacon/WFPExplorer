#include "pch.h"
#include "StringHelper.h"
#include <WFPEngine.h>

CString StringHelper::GuidToString(GUID const& guid) {
	WCHAR sguid[64];
	return ::StringFromGUID2(guid, sguid, _countof(sguid)) ? sguid : L"";
}

CString StringHelper::WFPValueToString(WFPValue const& value, bool hex) {
	CString str;
	switch (value.Type) {
		case WFPDataType::INT8: str.Format(hex ? L"0x%X" : L"%d", (int)value.int8); break;
		case WFPDataType::UINT8: str.Format(hex ? L"0x%X" : L"%u", (uint32_t)value.uint8); break;
		case WFPDataType::INT16: str.Format(hex ? L"0x%X" : L"%d", (int)value.int16); break;
		case WFPDataType::INT32: str.Format(hex ? L"0x%X" : L"%d", value.int32); break;
		case WFPDataType::UINT16: str.Format(hex ? L"0x%X" : L"%u", (uint32_t)value.uint16); break;
		case WFPDataType::UINT32: str.Format(hex ? L"0x%X" : L"%u", value.uint32); break;
		case WFPDataType::INT64: str.Format(hex ? L"0x%llX" : L"%lld", value.int64); break;
		case WFPDataType::UINT64: str.Format(hex ? L"0x%llX" : L"%llu", value.uint64); break;
		case WFPDataType::FLOAT: str.Format(L"%f", value.float32); break;
		case WFPDataType::DOUBLE: str.Format(L"%lf", value.double64); break;
	}
	return str;
}

CString StringHelper::WFPFilterFlagsToString(WFPFilterFlags flags) {
	static const struct {
		WFPFilterFlags flag;
		PCWSTR text;
	} data[] = {
		{ WFPFilterFlags::Persistent, L"Persistent" },
		{ WFPFilterFlags::BootTime, L"Boot Time" },
		{ WFPFilterFlags::HasProviderContext, L"Provider Context" },
		{ WFPFilterFlags::ClearActionRight, L"Clear Acyion Right" },
		{ WFPFilterFlags::PermitIfCalloutUnregistered, L"Permit if Callout Unregistered" },
		{ WFPFilterFlags::Disabled, L"Disabled" },
		{ WFPFilterFlags::Indexed, L"Indexed" },
		{ WFPFilterFlags::HasSecurityRealmProviderContext, L"Security Realm Provider Context" },
		{ WFPFilterFlags::SystemOsOnly, L"System OS Only" },
		{ WFPFilterFlags::GameOsOnly, L"Game OS Only" },
		{ WFPFilterFlags::SilentMode, L"Silent Mode" },
		{ WFPFilterFlags::IPSecNoAcquireInitiate, L"IPSec No Acquire Initiate" },
	};

	return FlagsToString(flags, data);
}

CString StringHelper::WFPLayerFlagsToString(WFPLayerFlags flags) {
	static const struct {
		WFPLayerFlags flag;
		PCWSTR text;
	} data[] = {
		{ WFPLayerFlags::Kernel, L"Kernel" },
		{ WFPLayerFlags::BuiltIn, L"Builtin" },
		{ WFPLayerFlags::ClassifyMostly, L"Class Modify" },
		{ WFPLayerFlags::Buffered, L"Buffered" },
	};
	return FlagsToString(flags, data);
}

