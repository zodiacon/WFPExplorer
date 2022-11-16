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

CString StringHelper::WFPSessionFlagsToString(WFPSessionFlags flags) {
	static const struct {
		WFPSessionFlags flag;
		PCWSTR text;
	} data[] = {
		{ WFPSessionFlags::Dynamic, L"Dynamic" },
	};

	return FlagsToString(flags, data);
}

CString StringHelper::WFPProviderFlagsToString(WFPProviderFlags flags) {
	static const struct {
		WFPProviderFlags flag;
		PCWSTR text;
	} data[] = {
		{ WFPProviderFlags::Persistent, L"Persistent" },
		{ WFPProviderFlags::Disabled, L"Disabled" },
	};

	return FlagsToString(flags, data);
}

CString StringHelper::WFPSubLayerFlagsToString(WFPSubLayerFlags flags) {
	static const struct {
		WFPSubLayerFlags flag;
		PCWSTR text;
	} data[] = {
		{ WFPSubLayerFlags::Persistent, L"Persistent" },
	};

	return FlagsToString(flags, data);
}

CString StringHelper::WFPCalloutFlagsToString(WFPCalloutFlags flags) {
	static const struct {
		WFPCalloutFlags flag;
		PCWSTR text;
	} data[] = {
		{ WFPCalloutFlags::Persistent, L"Persistent" },
		{ WFPCalloutFlags::UsesProviderContext, L"Provider Context" },
		{ WFPCalloutFlags::Registered, L"Registered" },
	};

	return FlagsToString(flags, data);
}

CString StringHelper::WFPProviderContextFlagsToString(WFPProviderContextFlags flags) {
	static const struct {
		WFPProviderContextFlags flag;
		PCWSTR text;
	} data[] = {
		{ WFPProviderContextFlags::Persistent, L"Persistent" },
		{ WFPProviderContextFlags::DownLevel, L"Downlevel" },
	};

	return FlagsToString(flags, data);
}

PCWSTR StringHelper::WFPProviderContextTypeToString(WFPProviderContextType type) {
	static PCWSTR types[] = {
		L"IPSec Keying",
		L"IPSec Ike Quick Mode Transport",
		L"IPSec Ike Quick ModeTunnel",
		L"IPSec AuthIP Quick Mode Transport",
		L"IPSec AuthIP Quick ModeTunnel",
		L"IPSec Ike Main Mode",
		L"IPSec AuthIP Main Mode",
		L"Classify Options",
		L"General",
		L"IPSec IkeV2 Quick Mode Tunnel",
		L"IPSec IKeV2 Main Mode",
		L"IPSec Dos Protection",
		L"IPSec IkeV2 Quick Mode Transport",
	};
	ATLASSERT((int)type < _countof(types));
	return types[(int)type];
}

PCWSTR StringHelper::WFPFilterActionTypeToString(WFPActionType const& type) {
	switch (type) {
		case WFPActionType::Block: return L"Block";
		case WFPActionType::Permit: return L"Permit";
		case WFPActionType::CalloutTerminating: return L"Callout Terminating";
		case WFPActionType::CalloutInspection: return L"Callout Inspection";
		case WFPActionType::CalloutUnknown: return L"Callout Unknown";
		case WFPActionType::Continue: return L"Continue";
		case WFPActionType::None: return L"None";
		case WFPActionType::NoneNoMatch: return L"None (No Match)";
	}
	return PCWSTR();
}
