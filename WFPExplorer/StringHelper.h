#pragma once

struct WFPValue;
enum class WFPFilterFlags;
enum class WFPLayerFlags;

struct StringHelper abstract final {
	static CString GuidToString(GUID const& guid);
	static CString WFPValueToString(WFPValue const& value, bool hex = false);
	static CString WFPFilterFlagsToString(WFPFilterFlags flags);
	static CString WFPLayerFlagsToString(WFPLayerFlags flags);

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

