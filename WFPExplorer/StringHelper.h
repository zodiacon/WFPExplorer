#pragma once

struct StringHelper abstract final {
	static CString GuidToString(GUID const& guid);
	static CString ParseMUIString(PCWSTR input);
	static CString WFPValueToString(FWP_VALUE const& value, bool hex = false, bool full = false);
	static CString WFPConditionValueToString(FWP_CONDITION_VALUE const& value, bool hex = false, bool full = false);
	static CString WFPFilterFlagsToString(UINT32 flags);
	static CString WFPLayerFlagsToString(UINT32 flags);
	static CString WFPSessionFlagsToString(UINT32 flags);
	static CString WFPProviderFlagsToString(DWORD flags);
	static CString WFPSubLayerFlagsToString(UINT32 flags);
	static CString WFPCalloutFlagsToString(DWORD flags);
	static CString WFPProviderContextFlagsToString(UINT32 flags);
	static PCWSTR NetEventTypeToString(FWPM_NET_EVENT_TYPE type);
	static PCWSTR WFPProviderContextTypeToString(FWPM_PROVIDER_CONTEXT_TYPE type);
	static PCWSTR WFPFilterActionTypeToString(FWP_ACTION_TYPE type);
	static PCWSTR WFPConditionMatchToString(FWP_MATCH_TYPE type);
	static PCWSTR WFPDataTypeToString(FWP_DATA_TYPE type);
	static CString WFPConditionFieldKeyToString(GUID const& key);
	static CString FormatBinary(BYTE const* buffer, ULONG size, int lineSize = 16, bool ascii = false);
	static CString FormatSID(PSID const sid);
	static PCWSTR WFPFieldTypeToString(FWPM_FIELD_TYPE type);
	static CString FormatIpv4Address(UINT32 address);
	static CString FormatIpv6Address(BYTE const* address);
	static PCWSTR IpProtocolToString(UINT8 protocol);
	static PCWSTR AddressFamilyToString(FWP_AF af);

	template<typename TFlags, typename TValue>
	static CString FlagsToString(TValue value, TFlags const& data) {
		CString result;
		for (auto& flag : data) {
			if (value == (TValue)0)
				break;
			if ((flag.flag & value) == flag.flag) {
				result += flag.text + CString(L", ");
				value &= ~flag.flag;
			}
		}
		return result.IsEmpty() ? result : result.Left(result.GetLength() - 2);
	}
};

