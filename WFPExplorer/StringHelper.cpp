#include "pch.h"
#include "StringHelper.h"
#include <WFPEngine.h>
#include <ip2string.h>

#pragma comment(lib, "ntdll")

CString StringHelper::GuidToString(GUID const& guid) {
	WCHAR sguid[64];
	return ::StringFromGUID2(guid, sguid, _countof(sguid)) ? sguid : L"";
}

CString StringHelper::WFPValueToString(WFPValue const& value, bool hex, bool full) {
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
		case WFPDataType::UNICODE_STRING_TYPE: return value.unicodeString;
		case WFPDataType::BYTE_ARRAY6_TYPE: return FormatBinary(value.byteArray6, 6);
		case WFPDataType::BYTE_ARRAY16_TYPE: return FormatBinary(value.byteArray16, 16);
		case WFPDataType::BYTE_BLOB_TYPE: return std::format(L"({} bytes) \r\n", value.byteBlob->size).c_str() +
			FormatBinary(value.byteBlob->data, full ? value.byteBlob->size : min(16, value.byteBlob->size));
		case WFPDataType::SID: return FormatSID(value.sid);
		case WFPDataType::SECURITY_DESCRIPTOR_TYPE:
			PWSTR sddl;
			if (::ConvertSecurityDescriptorToStringSecurityDescriptor(value.byteBlob->data, SDDL_REVISION_1,
				DACL_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION, &sddl, nullptr)) {
				str = sddl;
				::LocalFree(sddl);
			}
			break;
		case WFPDataType::RANGE_TYPE:
			return WFPValueToString(value.rangeValue->Low, hex, full) + L"\r\n to \r\n" + WFPValueToString(value.rangeValue->High, hex, full);

		case WFPDataType::V4_ADDR_MASK:
		{
			WCHAR buf[128];
			str = CString(L"IP: ") + ::RtlIpv4AddressToString((in_addr const*)&value.v4AddrMask->addr, buf) +
				L"Mask: " + ::RtlIpv4AddressToString((in_addr const*)&value.v4AddrMask->mask, buf);
			break;
		}

		case WFPDataType::V6_ADDR_MASK:
			WCHAR buf[256];
			str = CString(L"IP: ") + ::RtlIpv6AddressToString((in6_addr const*)&value.v6AddrMask->addr, buf) +
				L"Prefix: " + std::format(L"{}", value.v6AddrMask->prefixLength).c_str();
			break;

		case WFPDataType::EMPTY:
			return L"(Empty)";
	}
	return str;
}

CString StringHelper::WFPFilterFlagsToString(WFPFilterFlags flags) {
	static std::unordered_map<WFPFilterFlags, CString> cache;
	if (auto it = cache.find(flags); it != cache.end())
		return it->second;

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

	auto result = FlagsToString(flags, data);
	cache.insert({ flags, result });
	return result;
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

CString StringHelper::WFPProviderFlagsToString(DWORD flags) {
	static const struct {
		DWORD flag;
		PCWSTR text;
	} data[] = {
		{ FWPM_PROVIDER_FLAG_PERSISTENT, L"Persistent" },
		{ FWPM_PROVIDER_FLAG_DISABLED, L"Disabled" },
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

PCWSTR StringHelper::WFPConditionMatchToString(WFPMatchType type) {
	switch(type) {
		case WFPMatchType::Equal: return L"Equal";
		case WFPMatchType::Greater: return L"Greater";
		case WFPMatchType::Less: return L"Less";
		case WFPMatchType::GreaterOrEqual: return L"Greater or Equal";
		case WFPMatchType::LessOrEqual: return L"Less or Equal";
		case WFPMatchType::Range: return L"Range";
		case WFPMatchType::FlagsAllSet: return L"Flags All Set";
		case WFPMatchType::FlagsAnySet: return L"Flags Any Set";
		case WFPMatchType::FlagsNoneSet: return L"Flags Non Set";
		case WFPMatchType::EqualCaseInsensitive: return L"Equal Case Insensitive";
		case WFPMatchType::NotEqual: return L"Not Equal";
		case WFPMatchType::Prefix: return L"Prefix";
		case WFPMatchType::NotPrefix: return L"Not Prefix";
	}
	ATLASSERT(false);
	return L"";
}

PCWSTR StringHelper::WFPDataTypeToString(WFPDataType type) {
	switch (type) {
		case WFPDataType::UINT8: return L"UINT8";
		case WFPDataType::UINT16: return L"UINT16";
		case WFPDataType::UINT32: return L"UINT32";
		case WFPDataType::UINT64: return L"UINT64";
		case WFPDataType::INT8: return L"INT8";
		case WFPDataType::INT16: return L"INT16";
		case WFPDataType::INT32: return L"INT32";
		case WFPDataType::INT64: return L"INT64";
		case WFPDataType::FLOAT: return L"FLOAT";
		case WFPDataType::DOUBLE: return L"DOUBLE";
		case WFPDataType::BYTE_BLOB_TYPE: return L"BLOB";
		case WFPDataType::BYTE_ARRAY6_TYPE: return L"BYTE[6]";
		case WFPDataType::RANGE_TYPE: return L"Range";
		case WFPDataType::BYTE_ARRAY16_TYPE: return L"BYTE[16]";
		case WFPDataType::SID: return L"SID";
		case WFPDataType::TOKEN_INFORMATION_TYPE: return L"Token Info";
		case WFPDataType::TOKEN_ACCESS_INFORMATION_TYPE: return L"Token Access";
		case WFPDataType::SECURITY_DESCRIPTOR_TYPE: return L"SD";
		case WFPDataType::UNICODE_STRING_TYPE: return L"Unicode String";
		case WFPDataType::V4_ADDR_MASK: return L"IPV4 Address & MAsk";
		case WFPDataType::V6_ADDR_MASK: return L"IPV6 Address & MAsk";
		case WFPDataType::BITMAP_INDEX_TYPE: return L"Bitmap Index";
		case WFPDataType::BITMAP_ARRAY64_TYPE: return L"Bitmap Array";
		default:
			ATLASSERT(false);
			break;
	}
	return L"";
}

namespace std {
	template<>
	struct hash<GUID> {
		size_t operator()(GUID const& guid) const {
			return guid.Data1 ^ (guid.Data2 << 16) | ((size_t)guid.Data3 << 32);
		}

	};
}

CString StringHelper::WFPConditionFieldKeyToString(GUID const& key) {
	static const std::unordered_map<GUID, CString> fields {
		{ FWPM_CONDITION_SOURCE_INTERFACE_INDEX, L"Source Interface Index" },
		{ FWPM_CONDITION_SUB_INTERFACE_INDEX, L"Subinterface Index" },
		{ FWPM_CONDITION_TUNNEL_TYPE, L"Tunnel Type" },
		{ FWPM_CONDITION_INTERFACE_TYPE, L"Interface Type" },
		{ FWPM_CONDITION_INTERFACE_QUARANTINE_EPOCH, L"Interface Qurantine Epoch" },
		{ FWPM_CONDITION_IP_PHYSICAL_NEXTHOP_INTERFACE, L"IP Physical Next Hop Interface" },
		{ FWPM_CONDITION_IP_PHYSICAL_ARRIVAL_INTERFACE, L"IP Physical Arrival Interface" },
		{ FWPM_CONDITION_ORIGINAL_ICMP_TYPE, L"Original ICMP Type" },
		{ FWPM_CONDITION_REAUTHORIZE_REASON, L"Reauthorize Reason" },
		{ FWPM_CONDITION_NEXTHOP_INTERFACE_PROFILE_ID, L"Next Hop Interface Profile ID" },
		{ FWPM_CONDITION_ARRIVAL_INTERFACE_PROFILE_ID, L"Arrival Interface Profile ID" },
		{ FWPM_CONDITION_LOCAL_INTERFACE_PROFILE_ID, L"Local Interface Profile ID" },
		{ FWPM_CONDITION_CURRENT_PROFILE_ID, L"Current Profile ID" },
		{ FWPM_CONDITION_ORIGINAL_PROFILE_ID, L"Original Profile ID" },
		{ FWPM_CONDITION_NEXTHOP_INTERFACE_INDEX, L"Next Hop Interface Index" },
		{ FWPM_CONDITION_NEXTHOP_TUNNEL_TYPE, L"Next Hop Tunnel Type" },
		{ FWPM_CONDITION_NEXTHOP_INTERFACE_TYPE, L"Next Hop Interface Type" },
		{ FWPM_CONDITION_IP_NEXTHOP_INTERFACE, L"IP Next Hop Interface" },
		{ FWPM_CONDITION_NEXTHOP_SUB_INTERFACE_INDEX, L"Next Hop Subinterface Index" },
		{ FWPM_CONDITION_ARRIVAL_INTERFACE_INDEX, L"Arrival Interface Index" },
		{ FWPM_CONDITION_ARRIVAL_TUNNEL_TYPE, L"Arrival Tunnel Type" },
		{ FWPM_CONDITION_ARRIVAL_INTERFACE_TYPE, L"Arrival Interface Type" },
		{ FWPM_CONDITION_IP_ARRIVAL_INTERFACE, L"IP Arrival Interface" },
		{ FWPM_CONDITION_IP_LOCAL_INTERFACE, L"IP Local Interface" },
		{ FWPM_CONDITION_IP_NEXTHOP_ADDRESS, L"IP Next Hop Address" },
		{ FWPM_CONDITION_IP_DESTINATION_ADDRESS_TYPE, L"IP Destination Address Type" },
		{ FWPM_CONDITION_IP_LOCAL_ADDRESS_TYPE, L"IP Local Address Type" },
		{ FWPM_CONDITION_IP_DESTINATION_ADDRESS, L"IP Destination Address" },
		{ FWPM_CONDITION_IP_SOURCE_ADDRESS, L"IP Source Address" },
		{ FWPM_CONDITION_IP_REMOTE_ADDRESS, L"IP Remote Address" },
		{ FWPM_CONDITION_IP_LOCAL_ADDRESS, L"IP Local Address" },
		{ FWPM_CONDITION_ALE_EFFECTIVE_NAME, L"ALE Effective Name" },
		{ FWPM_CONDITION_IPSEC_SECURITY_REALM_ID, L"IPSec Security Realm ID" },
		{ FWPM_CONDITION_INTERFACE_MAC_ADDRESS, L"Interface MAC Address" },
		{ FWPM_CONDITION_MAC_LOCAL_ADDRESS, L"MAC Local Address" },
		{ FWPM_CONDITION_MAC_REMOTE_ADDRESS, L"MAC Remote Address" },
		{ FWPM_CONDITION_ETHER_TYPE, L"Ether Type" },
		{ FWPM_CONDITION_VLAN_ID, L"VLAN ID" },
		{ FWPM_CONDITION_VSWITCH_TENANT_NETWORK_ID, L"V-Switch Tenant Network ID" },
		{ FWPM_CONDITION_NDIS_PORT, L"NDIS Port" },
		{ FWPM_CONDITION_NDIS_MEDIA_TYPE, L"NDIS Media Type" },
		{ FWPM_CONDITION_NDIS_PHYSICAL_MEDIA_TYPE, L"NDIS Physical Media Type" },
		{ FWPM_CONDITION_L2_FLAGS, L"L2 Flags" },
		{ FWPM_CONDITION_MAC_LOCAL_ADDRESS_TYPE, L"MAC Local Address Type" },
		{ FWPM_CONDITION_MAC_REMOTE_ADDRESS_TYPE, L"MAC Remote Address Type" },
		{ FWPM_CONDITION_ALE_PACKAGE_ID, L"ALE Package ID" },
		{ FWPM_CONDITION_MAC_SOURCE_ADDRESS, L"MAC Source Address" },
		{ FWPM_CONDITION_MAC_DESTINATION_ADDRESS, L"MAC Destination Address" },
		{ FWPM_CONDITION_MAC_SOURCE_ADDRESS_TYPE, L"MAC Source Address Type" },
		{ FWPM_CONDITION_MAC_DESTINATION_ADDRESS_TYPE, L"MAC Destination Address Type" },
		{ FWPM_CONDITION_IP_SOURCE_PORT, L"IP Source Port" },
		{ FWPM_CONDITION_IP_DESTINATION_PORT, L"IP Destination Port" },
		{ FWPM_CONDITION_VSWITCH_ID, L"V-Switch ID" },
		{ FWPM_CONDITION_VSWITCH_NETWORK_TYPE, L" V-Switch Network Type" },
		{ FWPM_CONDITION_FLAGS, L"Flags" },
		{ FWPM_CONDITION_ALE_APP_ID, L"ALE App ID" },
		{ FWPM_CONDITION_DESTINATION_INTERFACE_INDEX, L"Destination Interface Index" },
		{ FWPM_CONDITION_DESTINATION_SUB_INTERFACE_INDEX, L"Destination Subinterface Index" },
		{ FWPM_CONDITION_ALE_ORIGINAL_APP_ID, L"ALE Original App ID" },
		{ FWPM_CONDITION_ALE_USER_ID, L"ALE User ID" },
		{ FWPM_CONDITION_ALE_REMOTE_USER_ID, L"ALE Remote User ID" },
		{ FWPM_CONDITION_ALE_REMOTE_MACHINE_ID, L"ALE Remote Machine ID" },
		{ FWPM_CONDITION_ALE_PROMISCUOUS_MODE, L"ALE Promiscuous Mode" },
		{ FWPM_CONDITION_ALE_SIO_FIREWALL_SYSTEM_PORT, L"ALE SI/O Firewall System Port" },
		{ FWPM_CONDITION_ALE_REAUTH_REASON, L"ALE Reauth Reason" },
		{ FWPM_CONDITION_ALE_NAP_CONTEXT, L"ALE NAP Context" },
		{ FWPM_CONDITION_KM_AUTH_NAP_CONTEXT, L"KM ALE NAP Context" },
		{ FWPM_CONDITION_REMOTE_USER_TOKEN, L"Remote User Token" },
		{ FWPM_CONDITION_RPC_IF_UUID, L"RPC Interface UUID" },
		{ FWPM_CONDITION_RPC_IF_VERSION, L"RPC Interface Version" },
		{ FWPM_CONDITION_RPC_IF_FLAG, L"RPC Interface Flag" },
		{ FWPM_CONDITION_DCOM_APP_ID, L"DCOM App ID" },
		{ FWPM_CONDITION_IMAGE_NAME, L"Image Name" },
		{ FWPM_CONDITION_RPC_PROTOCOL, L"RPC Protocol" },
		{ FWPM_CONDITION_RPC_AUTH_TYPE, L"RPC Auth Type" },
		{ FWPM_CONDITION_RPC_AUTH_LEVEL, L"RPC Auth Level" },
		{ FWPM_CONDITION_SEC_ENCRYPT_ALGORITHM, L"Security Encryption Algorithm" },
		{ FWPM_CONDITION_SEC_KEY_SIZE, L"Security Key Size" },
		{ FWPM_CONDITION_IP_LOCAL_ADDRESS_V4, L"IP Local Address V4" },
		{ FWPM_CONDITION_IP_LOCAL_ADDRESS_V6, L"IP Local Address V6" },
		{ FWPM_CONDITION_PIPE, L"Pipe" },
		{ FWPM_CONDITION_IP_REMOTE_ADDRESS_V4, L"IP Remote Address V4" },
		{ FWPM_CONDITION_IP_REMOTE_ADDRESS_V6, L"IP Remote Address V6" },
		{ FWPM_CONDITION_PROCESS_WITH_RPC_IF_UUID, L"Process with RPC Interface UUID" },
		{ FWPM_CONDITION_RPC_EP_VALUE, L"RPC EP Value" },
		{ FWPM_CONDITION_RPC_EP_FLAGS, L"RPC EP Flags" },
		{ FWPM_CONDITION_CLIENT_TOKEN, L"Client Token" },
		{ FWPM_CONDITION_RPC_SERVER_NAME, L"RPC Server Name" },
		{ FWPM_CONDITION_RPC_SERVER_PORT, L"RPC Server Port" },
		{ FWPM_CONDITION_RPC_PROXY_AUTH_TYPE, L"RPC Proxy Auth Type" },
		{ FWPM_CONDITION_CLIENT_CERT_KEY_LENGTH, L"Client Certificate Key Length" },
		{ FWPM_CONDITION_CLIENT_CERT_OID, L"Client Certificate OID" },
		{ FWPM_CONDITION_NET_EVENT_TYPE, L"NET Event Type" },
		{ FWPM_CONDITION_PEER_NAME, L"Peer Name" },
		{ FWPM_CONDITION_REMOTE_ID, L"Remote ID" },
		{ FWPM_CONDITION_AUTHENTICATION_TYPE, L"Authentication Type" },
		{ FWPM_CONDITION_KM_TYPE, L"KM Type" },
		{ FWPM_CONDITION_KM_MODE, L"KM Mode" },
		{ FWPM_CONDITION_IPSEC_POLICY_KEY, L"IPSec Policy Key" },
		{ FWPM_CONDITION_QM_MODE, L"QM Mode" },
		{ FWPM_CONDITION_COMPARTMENT_ID, L"Compartment ID" },
		{ FWPM_CONDITION_IP_PROTOCOL, L"IP Protocol" },
		{ FWPM_CONDITION_IP_FORWARD_INTERFACE, L"IP Forward Interface" },
		{ FWPM_CONDITION_IP_REMOTE_PORT, L"IP Remote Port" },
		{ FWPM_CONDITION_IP_LOCAL_PORT, L"IP Local Port" },
		{ FWPM_CONDITION_EMBEDDED_LOCAL_ADDRESS_TYPE, L"Embedded Local Address Type" },
		{ FWPM_CONDITION_EMBEDDED_REMOTE_ADDRESS, L"Embedded Remote Address" },
		{ FWPM_CONDITION_EMBEDDED_PROTOCOL, L"Embedded Protocol" },
		{ FWPM_CONDITION_EMBEDDED_LOCAL_PORT, L"Embedded Local Port" },
		{ FWPM_CONDITION_EMBEDDED_REMOTE_PORT, L"Embedded Remote Port" },
		{ FWPM_CONDITION_DIRECTION, L"Direction" },
		{ FWPM_CONDITION_INTERFACE_INDEX, L"Interface Index" },
		{ FWPM_CONDITION_SOURCE_SUB_INTERFACE_INDEX, L"Source Subinterface Index" },
		{ FWPM_CONDITION_ALE_SECURITY_ATTRIBUTE_FQBN_VALUE, L"ALE Security Attribute FQBN Value" },
	};
	if (auto it = fields.find(key); it != fields.end())
		return it->second;

	return GuidToString(key);
}

CString StringHelper::FormatBinary(BYTE const* buffer, ULONG size, int lineSize) {
	CString text;
	for (ULONG i = 0; i < size; i++) {
		text += std::format(L"{:02X} ", buffer[i]).c_str();
		if (i % lineSize == lineSize - 1)
			text += L"\r\n";
	}
	return text;
}

CString StringHelper::FormatSID(PSID const sid) {
	WCHAR name[32], domain[32];
	DWORD lname = _countof(name), ldomain = _countof(domain);
	SID_NAME_USE use;
	if (::LookupAccountSid(nullptr, sid, name, &lname, domain, &ldomain, &use))
		return ldomain ? (domain + CString(L"\\") + name) : name;
	PWSTR ssid;
	if (::ConvertSidToStringSid(sid, &ssid)) {
		CString name(ssid);
		::LocalFree(ssid);
		return name;
	}
	return L"";
}

PCWSTR StringHelper::WFPFieldTypeToString(WFPFieldType type) {
	switch (type) {
		case WFPFieldType::RawData: return L"Raw Data";
		case WFPFieldType::IpAddress: return L"IP Address";
		case WFPFieldType::Flags: return L"Flags";
	}
	ATLASSERT(false);
	return L"";
}

PCWSTR StringHelper::WFPFilterActionTypeToString(WFPActionType type) {
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
