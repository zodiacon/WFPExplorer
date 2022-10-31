#include "pch.h"
#include "StringHelper.h"

CString StringHelper::GuidToString(GUID const& guid) {
	WCHAR sguid[64];
	return ::StringFromGUID2(guid, sguid, _countof(sguid)) ? sguid : L"";
}
