#include "pch.h"
#include "WFPHelper.h"
#include <WFPEngine.h>
#include "StringHelper.h"

CString WFPHelper::GetProviderName(WFPEngine const& engine, GUID const& key) {
	if (key != GUID_NULL) {
		auto provider = engine.GetProviderByKey(key);
		if (provider && !provider->Name.empty() && provider->Name[0] != L'@')
			return provider->Name.c_str();
		return StringHelper::GuidToString(key);
	}
	return L"";
}

CString WFPHelper::GetFilterName(WFPEngine const& engine, GUID const& key) {
	if (key != GUID_NULL) {
		auto filter = engine.GetFilterByKey(key);
		if (filter && !filter->Name.empty() && filter->Name[0] != L'@')
			return filter->Name.c_str();
		return StringHelper::GuidToString(key);
	}
	return L"";
}

CString WFPHelper::GetLayerName(WFPEngine const& engine, GUID const& key) {
	if (key != GUID_NULL) {
		auto layer = engine.GetLayerByKey(key);
		if (layer && !layer->Name.empty() && layer->Name[0] != L'@')
			return layer->Name.c_str();
		return StringHelper::GuidToString(key);
	}
	return L"";
}

CString WFPHelper::GetSublayerName(WFPEngine const& engine, GUID const& key) {
	if (key != GUID_NULL) {
		auto layer = engine.GetSublayerByKey(key);
		if (layer && !layer->Name.empty() && layer->Name[0] != L'@')
			return layer->Name.c_str();
		return StringHelper::GuidToString(key);
	}
	return L"";
}
