#pragma once

struct WFPValue;

struct StringHelper abstract final {
	static CString GuidToString(GUID const& guid);
	static CString WFPValueToString(WFPValue const& value, bool hex = false);
};

