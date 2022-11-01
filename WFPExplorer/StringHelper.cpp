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
		case WFPDataType::INT8: str.Format(L"%d", (int)value.int8); break;
		case WFPDataType::UINT8: str.Format(L"%u", (int)value.uint8); break;
		case WFPDataType::INT16: str.Format(L"%d", (int)value.int16); break;
		case WFPDataType::INT32: str.Format(L"%d", value.int32); break;
		case WFPDataType::UINT16: str.Format(hex ? L"0x%X" : L"%u", (uint32_t)value.uint16); break;
		case WFPDataType::UINT32: str.Format(hex ? L"0x%X" : L"%u", value.uint32); break;
		case WFPDataType::INT64: str.Format(hex ? L"0x%llX" : L"%lld", value.int64); break;
		case WFPDataType::UINT64: str.Format(hex ? L"0x%llX" : L"%llu", value.uint64); break;
		case WFPDataType::FLOAT: str.Format(L"%f", value.float32); break;
		case WFPDataType::DOUBLE: str.Format(L"%lf", value.double64); break;
	}
	return str;
}

