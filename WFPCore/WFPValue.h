#pragma once

#include <type_traits>

template<typename T = FWP_VALUE>
struct WFPValue : T {
	WFPValue() {
		this->type = FWP_EMPTY;
	}
	explicit WFPValue(int8_t value) {
		this->int8 = value; this->type = FWP_INT8;
	}
	explicit WFPValue(uint8_t value) {
		this->uint8 = value; this->type = FWP_UINT8;
	}
	explicit WFPValue(int16_t value) {
		this->int16 = value; this->type = FWP_INT16;
	}
	explicit WFPValue(uint16_t value) {
		this->uint16 = value; this->type = FWP_UINT16;
	}
	explicit WFPValue(int32_t value) {
		this->int32 = value; this->type = FWP_INT32;
	}
	explicit WFPValue(uint32_t value) {
		this->uint32 = value; this->type = FWP_UINT32;
	}
	explicit WFPValue(int64_t& value) {
		this->int64 = &value; this->type = FWP_INT64;
	}
	explicit WFPValue(uint64_t& value) {
		this->uint64 = &value; this->type = FWP_UINT64;
	}
	explicit WFPValue(float value) {
		this->float32 = value; this->type = FWP_FLOAT;
	}
	explicit WFPValue(double& value) {
		this->double64 = &value; this->type = FWP_DOUBLE;
	}
	explicit WFPValue(SID* sid) {
		this->sid = sid; this->type = FWP_SID;
	}

	explicit WFPValue(FWP_BYTE_BLOB* blob) {
		this->byteBlob = blob;
		this->type = FWP_BYTE_BLOB_TYPE;
	}

	explicit WFPValue(FWP_RANGE& range) {
		if constexpr (std::is_base_of_v<FWP_CONDITION_VALUE, T>) {
			this->type = FWP_RANGE_TYPE;
			this->rangeValue = &range;
		}
	}
};

using WFPConditionValue = WFPValue<FWP_CONDITION_VALUE>;
