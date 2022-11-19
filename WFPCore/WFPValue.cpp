#include "pch.h"
#include "WFPEngine.h"

WFPValue::WFPValue() : Type(WFPDataType::EMPTY) {
}

WFPValue::~WFPValue() {
	Free();
}

WFPValue::WFPValue(WFPValue const& other) {
	Init(other);
}

WFPValue::WFPValue(WFPValue&& other) noexcept {
	memcpy(this, &other, sizeof(other));
	other.Type = WFPDataType::EMPTY;
}

WFPValue& WFPValue::operator=(WFPValue const& other) {
	if (&other != this) {
		Free();
		Init(other);
	}
	return *this;
}

WFPValue& WFPValue::operator=(WFPValue&& other) noexcept {
	if (&other != this) {
		Free();
		memcpy(this, &other, sizeof(other));
		other.Type = WFPDataType::EMPTY;
	}
	return *this;
}

void WFPValue::Free() {
	switch (Type) {
		case WFPDataType::BYTE_ARRAY16_TYPE:
			delete[] byteArray16;
			break;

		case WFPDataType::UNICODE_STRING_TYPE:
			delete[] unicodeString;
			break;

		case WFPDataType::SID:
			::free(sid);
			break;

		case WFPDataType::SECURITY_DESCRIPTOR_TYPE:
		case WFPDataType::BYTE_BLOB_TYPE:
		case WFPDataType::TOKEN_ACCESS_INFORMATION_TYPE:
			delete[] byteBlob->data;
			delete byteBlob;
			break;

		case WFPDataType::RANGE_TYPE:
			delete rangeValue;
			break;

	}
	Type = WFPDataType::EMPTY;
}

WFPValue& WFPValue::Init(WFPValue const& value) {
	Type = value.Type;

	switch (Type) {
		case WFPDataType::BYTE_ARRAY16_TYPE:
			byteArray16 = new BYTE[16];
			memcpy(byteArray16, value.byteArray16, 16);
			break;

		case WFPDataType::SECURITY_DESCRIPTOR_TYPE:
		case WFPDataType::BYTE_BLOB_TYPE:
		case WFPDataType::TOKEN_ACCESS_INFORMATION_TYPE:
		{
			sd = new FWP_BYTE_BLOB;
			auto len = value.sd->size;
			sd->data = new BYTE[len];
			sd->size = len;
			memcpy(sd->data, value.sd->data, len);
			break;
		}

		case WFPDataType::SID:
		{
			ATLASSERT(value.sid);
			auto len = ::GetLengthSid(value.sid);
			sid = (SID*)::malloc(len);
			ATLASSERT(sid);
			::CopySid(len, sid, value.sid);
			break;
		}

		case WFPDataType::UNICODE_STRING_TYPE:
			ULONG len;
			unicodeString = new WCHAR[len = ULONG(wcslen(value.unicodeString) + 1)];
			wcscpy_s(unicodeString, len, value.unicodeString);
			break;

		case WFPDataType::RANGE_TYPE:
			rangeValue = new WFPRange(value.rangeValue->Low, value.rangeValue->High);
			break;

		default:
			memcpy(this, &value, sizeof(value));
	}
	return *this;
}

WFPRange::WFPRange(WFPValue low, WFPValue high) noexcept {
	Low = std::move(low);
	High = std::move(high);
}
