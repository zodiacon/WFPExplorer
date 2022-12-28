#include "pch.h"
#include "StringHelper.h"
#include <WFPEngine.h>
#include <ip2string.h>
#include <sddl.h>

#pragma comment(lib, "ntdll")
#pragma comment(lib, "ws2_32")

CString StringHelper::ParseMUIString(PCWSTR input) {
	if (input == nullptr)
		return L"";

	if (*input && input[0] == L'@') {
		WCHAR result[256];
		if (::SHLoadIndirectString(input, result, _countof(result), nullptr) == S_OK)
			return result;
	}
	return input;
}

CString StringHelper::GuidToString(GUID const& guid) {
	WCHAR sguid[64];
	return ::StringFromGUID2(guid, sguid, _countof(sguid)) ? sguid : L"";
}

CString StringHelper::WFPValueToString(FWP_VALUE const& value, bool hex, bool full) {
	CString str;
	switch (value.type) {
		case FWP_INT8: str.Format(hex ? L"0x%X" : L"%d", (int)value.int8); break;
		case FWP_UINT8: str.Format(hex ? L"0x%X" : L"%u", (uint32_t)value.uint8); break;
		case FWP_INT16: str.Format(hex ? L"0x%X" : L"%d", (int)value.int16); break;
		case FWP_INT32: str.Format(hex ? L"0x%X" : L"%d", value.int32); break;
		case FWP_UINT16: str.Format(hex ? L"0x%X" : L"%u", (uint32_t)value.uint16); break;
		case FWP_UINT32: str.Format(hex ? L"0x%X" : L"%u", value.uint32); break;
		case FWP_INT64: str.Format(hex ? L"0x%llX" : L"%lld", *value.int64); break;
		case FWP_UINT64: str.Format(hex ? L"0x%llX" : L"%llu", *value.uint64); break;
		case FWP_FLOAT: str.Format(L"%f", value.float32); break;
		case FWP_DOUBLE: str.Format(L"%lf", *value.double64); break;
		case FWP_UNICODE_STRING_TYPE: return value.unicodeString;
		case FWP_BYTE_ARRAY6_TYPE: return FormatBinary(value.byteArray6->byteArray6, 6);
		case FWP_BYTE_ARRAY16_TYPE: return FormatBinary(value.byteArray16->byteArray16, 16);
		case FWP_BYTE_BLOB_TYPE: return std::format(L"({} bytes) \r\n", value.byteBlob->size).c_str() +
			FormatBinary(value.byteBlob->data, full ? value.byteBlob->size : min(16, value.byteBlob->size), 16, full);
		case FWP_SID: return FormatSID(value.sid);
		case FWP_SECURITY_DESCRIPTOR_TYPE:
			PWSTR sddl;
			if (::ConvertSecurityDescriptorToStringSecurityDescriptor(value.byteBlob->data, SDDL_REVISION_1,
				DACL_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION, &sddl, nullptr)) {
				str = sddl;
				::LocalFree(sddl);
			}
			break;

		case FWP_EMPTY:
			return L"(Empty)";
	}
	return str;
}

CString StringHelper::WFPConditionValueToString(FWP_CONDITION_VALUE const& value, bool hex, bool full) {
	CString str;
	switch (value.type) {
		case FWP_RANGE_TYPE:
			return WFPValueToString(value.rangeValue->valueLow, hex, full) + L"\r\n to \r\n" + WFPValueToString(value.rangeValue->valueHigh, hex, full);

		case FWP_V4_ADDR_MASK:
		{
			WCHAR buf[128];
			str = CString(L"IP: ") + ::RtlIpv4AddressToString((in_addr const*)&value.v4AddrMask->addr, buf) +
				L"Mask: " + ::RtlIpv4AddressToString((in_addr const*)&value.v4AddrMask->mask, buf);
			break;
		}

		case FWP_V6_ADDR_MASK:
			WCHAR buf[256];
			str = CString(L"IP: ") + ::RtlIpv6AddressToString((in6_addr const*)&value.v6AddrMask->addr, buf) +
				L"Prefix: " + std::format(L"{}", value.v6AddrMask->prefixLength).c_str();
			break;

		default:
			return WFPValueToString(*(FWP_VALUE*)&value, hex, full);
	}
	return L"";
}

CString StringHelper::WFPFilterFlagsToString(UINT32 flags) {
	static std::unordered_map<UINT32, CString> cache;
	if (auto it = cache.find(flags); it != cache.end())
		return it->second;

	static const struct {
		UINT32 flag;
		PCWSTR text;
	} data[] = {
		{ FWPM_FILTER_FLAG_PERSISTENT,							L"Persistent" },
		{ FWPM_FILTER_FLAG_BOOTTIME,							L"Boot Time" },
		{ FWPM_FILTER_FLAG_HAS_PROVIDER_CONTEXT,				L"Provider Context" },
		{ FWPM_FILTER_FLAG_CLEAR_ACTION_RIGHT,					L"Clear Acyion Right" },
		{ FWPM_FILTER_FLAG_PERMIT_IF_CALLOUT_UNREGISTERED,		L"Permit if Callout Unregistered" },
		{ FWPM_FILTER_FLAG_DISABLED,							L"Disabled" },
		{ FWPM_FILTER_FLAG_INDEXED,								L"Indexed" },
		{ FWPM_FILTER_FLAG_HAS_SECURITY_REALM_PROVIDER_CONTEXT,	L"Security Realm Provider Context" },
		{ FWPM_FILTER_FLAG_SYSTEMOS_ONLY,						L"System OS Only" },
		{ FWPM_FILTER_FLAG_GAMEOS_ONLY,							L"Game OS Only" },
		{ FWPM_FILTER_FLAG_SILENT_MODE,							L"Silent Mode" },
		{ FWPM_FILTER_FLAG_IPSEC_NO_ACQUIRE_INITIATE,			L"IPSec No Acquire Initiate" },
	};

	auto result = FlagsToString(flags, data);
	cache.insert({ flags, result });
	return result;
}

CString StringHelper::WFPLayerFlagsToString(UINT32 flags) {
	static const struct {
		UINT32 flag;
		PCWSTR text;
	} data[] = {
		{ FWPM_LAYER_FLAG_KERNEL,			L"Kernel" },
		{ FWPM_LAYER_FLAG_BUILTIN,			L"Builtin" },
		{ FWPM_LAYER_FLAG_CLASSIFY_MOSTLY,	L"Class Mostly" },
		{ FWPM_LAYER_FLAG_BUFFERED,			L"Buffered" },
	};
	return FlagsToString(flags, data);
}

CString StringHelper::WFPSessionFlagsToString(UINT32 flags) {
	static const struct {
		UINT32 flag;
		PCWSTR text;
	} data[] = {
		{ FWPM_SESSION_FLAG_DYNAMIC, L"Dynamic" },
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

CString StringHelper::WFPSubLayerFlagsToString(UINT32 flags) {
	static const struct {
		UINT32 flag;
		PCWSTR text;
	} data[] = {
		{ FWPM_SUBLAYER_FLAG_PERSISTENT, L"Persistent" },
	};

	return FlagsToString(flags, data);
}

CString StringHelper::WFPCalloutFlagsToString(DWORD flags) {
	static const struct {
		DWORD flag;
		PCWSTR text;
	} data[] = {
		{ FWPM_CALLOUT_FLAG_PERSISTENT,				L"Persistent" },
		{ FWPM_CALLOUT_FLAG_REGISTERED,				L"Registered" },
		{ FWPM_CALLOUT_FLAG_USES_PROVIDER_CONTEXT,	L"Provider Context" },
	};

	return FlagsToString(flags, data);
}

CString StringHelper::WFPProviderContextFlagsToString(UINT32 flags) {
	static const struct {
		UINT32 flag;
		PCWSTR text;
	} data[] = {
		{ FWPM_PROVIDER_CONTEXT_FLAG_PERSISTENT, L"Persistent" },
		{ FWPM_PROVIDER_CONTEXT_FLAG_DOWNLEVEL, L"Downlevel" },
	};

	return FlagsToString(flags, data);
}

PCWSTR StringHelper::NetEventTypeToString(FWPM_NET_EVENT_TYPE type) {
	static PCWSTR types[] = {
		L"IKExt Main Mode Failure",
		L"IKExt Quick Mode Failure",
		L"IKExt Extended Mode Failure",
		L"Classify Drop",
		L"IPSec Kernel Drop",
		L"IPSec DoS Protection Drop",
		L"Classify Allow",
		L"Capability Drop",
		L"Capability Allow",
		L"Classify Drop MAC",
		L"LPM Packet Arrival",
	};
	ATLASSERT(type < _countof(types));
	return types[type];
}

PCWSTR StringHelper::WFPProviderContextTypeToString(FWPM_PROVIDER_CONTEXT_TYPE type) {
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

PCWSTR StringHelper::WFPConditionMatchToString(FWP_MATCH_TYPE type) {
	switch(type) {
		case FWP_MATCH_EQUAL:					return L"Equal";
		case FWP_MATCH_GREATER:					return L"Greater";
		case FWP_MATCH_LESS:					return L"Less";
		case FWP_MATCH_GREATER_OR_EQUAL:		return L"Greater or Equal";
		case FWP_MATCH_LESS_OR_EQUAL:			return L"Less or Equal";
		case FWP_MATCH_RANGE:					return L"Range";
		case FWP_MATCH_FLAGS_ALL_SET:			return L"Flags All Set";
		case FWP_MATCH_FLAGS_ANY_SET:			return L"Flags Any Set";
		case FWP_MATCH_FLAGS_NONE_SET:			return L"Flags Non Set";
		case FWP_MATCH_EQUAL_CASE_INSENSITIVE:	return L"Equal Case Insensitive";
		case FWP_MATCH_NOT_EQUAL:				return L"Not Equal";
		case FWP_MATCH_PREFIX:					return L"Prefix";
		case FWP_MATCH_NOT_PREFIX:				return L"Not Prefix";
	}
	ATLASSERT(false);
	return L"";
}

PCWSTR StringHelper::WFPDataTypeToString(FWP_DATA_TYPE type) {
	switch (type) {
		case FWP_UINT8: return L"UINT8";
		case FWP_UINT16: return L"UINT16";
		case FWP_UINT32: return L"UINT32";
		case FWP_UINT64: return L"UINT64";
		case FWP_INT8: return L"INT8";
		case FWP_INT16: return L"INT16";
		case FWP_INT32: return L"INT32";
		case FWP_INT64: return L"INT64";
		case FWP_FLOAT: return L"FLOAT";
		case FWP_DOUBLE: return L"DOUBLE";
		case FWP_BYTE_BLOB_TYPE: return L"BLOB";
		case FWP_BYTE_ARRAY6_TYPE: return L"BYTE[6]";
		case FWP_RANGE_TYPE: return L"Range";
		case FWP_BYTE_ARRAY16_TYPE: return L"BYTE[16]";
		case FWP_SID: return L"SID";
		case FWP_TOKEN_INFORMATION_TYPE: return L"Token Info";
		case FWP_TOKEN_ACCESS_INFORMATION_TYPE: return L"Token Access";
		case FWP_SECURITY_DESCRIPTOR_TYPE: return L"SD";
		case FWP_UNICODE_STRING_TYPE: return L"Unicode String";
		case FWP_V4_ADDR_MASK: return L"IPV4 Address & MAsk";
		case FWP_V6_ADDR_MASK: return L"IPV6 Address & MAsk";
		case FWP_BYTE_ARRAY6_TYPE + 1: return L"Bitmap Index";
		case FWP_BYTE_ARRAY6_TYPE + 2: return L"Bitmap Array";
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

CString StringHelper::FormatBinary(BYTE const* buffer, ULONG size, int lineSize, bool ascii) {
	CString text;
	CString asciiText;
	for (ULONG i = 0; i < size; i++) {
		text += std::format(L"{:02X} ", buffer[i]).c_str();
		if (ascii)
			asciiText += isprint(buffer[i]) ? (WCHAR)buffer[i] : L'.';
		if (i % lineSize == lineSize - 1) {
			if (ascii) {
				text += L"  " + asciiText;
				asciiText.Empty();
			}
			text += L"\r\n";
		}
	}
	if(ascii && !asciiText.IsEmpty())
		text += std::wstring(3 * (lineSize - size % lineSize) + 2, L' ').c_str() + asciiText;
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

PCWSTR StringHelper::WFPFieldTypeToString(FWPM_FIELD_TYPE type) {
	switch (type) {
		case FWPM_FIELD_RAW_DATA: return L"Raw Data";
		case FWPM_FIELD_IP_ADDRESS: return L"IP Address";
		case FWPM_FIELD_FLAGS: return L"Flags";
	}
	ATLASSERT(false);
	return L"";
}

CString StringHelper::FormatIpv4Address(UINT32 address) {
	WCHAR buf[32];
	auto haddr = ntohl(address);
	::RtlIpv4AddressToString((in_addr const*)&haddr, buf);
	return buf;
}

CString StringHelper::FormatIpv6Address(BYTE const* address) {
	WCHAR buf[64];
	::RtlIpv6AddressToString((in6_addr const*)address, buf);
	return buf;
}

PCWSTR StringHelper::IpProtocolToString(UINT8 protocol) {
	switch (protocol) {
		case IPPROTO_HOPOPTS: return L"IPv6 HopxHop Opts";
		case IPPROTO_ICMP: return L"ICMP";
		case IPPROTO_IGMP: return L"IGMP";
		case IPPROTO_GGP: return L"GGP";
		case IPPROTO_IPV4: return L"IPv4";
		case IPPROTO_ST: return L"ST";
		case IPPROTO_TCP: return L"TCP";
		case IPPROTO_CBT: return L"CBT";
		case IPPROTO_EGP: return L"EGP";
		case IPPROTO_IGP: return L"IGP";
		case IPPROTO_PUP: return L"PUP";
		case IPPROTO_UDP: return L"UDP";
		case IPPROTO_IDP: return L"IDP";
		case IPPROTO_RDP: return L"RDP";
		case IPPROTO_IPV6: return L"IPv6";
		case IPPROTO_ROUTING: return L"Routing";
		case IPPROTO_FRAGMENT: return L"Fragment";
		case IPPROTO_ESP: return L"ESP";
		case IPPROTO_AH: return L"AH";
		case IPPROTO_ICMPV6: L"ICMPv6";
		case IPPROTO_NONE: return L"IPv6 NNH";
		case IPPROTO_DSTOPTS: return L"IPv6 Dst Opts";
		case IPPROTO_ND: return L"ND";
		case IPPROTO_ICLFXBM: return L"ICLFXBM";
		case IPPROTO_PIM: return L"PIM";
		case IPPROTO_PGM: return L"PGM";
		case IPPROTO_L2TP: return L"L2TP";
		case IPPROTO_SCTP: return L"SCTP";
		case IPPROTO_RAW: return L"RAW";
	};
	return L"(Unknown)";
}

PCWSTR StringHelper::AddressFamilyToString(FWP_AF af) {
	switch (af) {
		case FWP_AF_INET: return L"INET";
		case FWP_AF_INET6: return L"INET6";
		case FWP_AF_ETHER: return L"ETHER";
	}
	return L"(Unknown)";
}

PCWSTR StringHelper::WFPFilterActionTypeToString(FWP_ACTION_TYPE type) {
	switch (type) {
		case FWP_ACTION_BLOCK:					return L"Block";
		case FWP_ACTION_PERMIT:					return L"Permit";
		case FWP_ACTION_CALLOUT_TERMINATING:	return L"Callout Terminating";
		case FWP_ACTION_CALLOUT_INSPECTION:		return L"Callout Inspection";
		case FWP_ACTION_CALLOUT_UNKNOWN:		return L"Callout Unknown";
		case FWP_ACTION_CONTINUE:				return L"Continue";
		case FWP_ACTION_NONE:					return L"None";
		case FWP_ACTION_NONE_NO_MATCH:			return L"None (No Match)";
	}
	return L"";
}
