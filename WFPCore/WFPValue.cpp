#include "pch.h"
#include "WFPEngine.h"

WFPValue::~WFPValue() {
	switch (Type) {
		case WFPDataType::BYTE_ARRAY16_TYPE:
			delete[] byteArray16;
			break;

		case WFPDataType::UNICODE_STRING_TYPE:
			delete[] unicodeString;
			break;

		case WFPDataType::SID:
			::LocalFree(sid);
			break;

		case WFPDataType::BYTE_BLOB_TYPE:
			delete[] byteBlob->data;
			break;
	}
}