#pragma once

struct WFPValue;
enum class WFPFilterFlags;
enum class WFPLayerFlags;
enum class WFPSessionFlags;
enum class WFPProviderFlags;
enum class WFPSubLayerFlags;
enum class WFPCalloutFlags;
enum class WFPProviderContextFlags;
enum class WFPProviderContextType;
enum class WFPActionType;
enum class WFPMatchType;
enum class WFPDataType;

struct StringHelper abstract final {
	static CString GuidToString(GUID const& guid);
	static CString WFPValueToString(WFPValue const& value, bool hex = false, bool full = false);
	static CString WFPFilterFlagsToString(WFPFilterFlags flags);
	static CString WFPLayerFlagsToString(WFPLayerFlags flags);
	static CString WFPSessionFlagsToString(WFPSessionFlags flags);
	static CString WFPProviderFlagsToString(WFPProviderFlags flags);
	static CString WFPSubLayerFlagsToString(WFPSubLayerFlags flags);
	static CString WFPCalloutFlagsToString(WFPCalloutFlags flags);
	static CString WFPProviderContextFlagsToString(WFPProviderContextFlags flags);
	static PCWSTR WFPProviderContextTypeToString(WFPProviderContextType type);
	static PCWSTR WFPFilterActionTypeToString(WFPActionType type);
	static PCWSTR WFPConditionMatchToString(WFPMatchType type);
	static PCWSTR WFPDataTypeToString(WFPDataType type);
	static CString WFPConditionFieldKeyToString(GUID const& key);
	static CString FormatBinary(BYTE const* buffer, ULONG size, int lineSize = 16);
	static CString FormatSID(PSID const sid);

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

