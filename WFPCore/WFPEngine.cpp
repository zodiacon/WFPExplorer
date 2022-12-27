#include "pch.h"
#include "WFPEngine.h"
#include <unordered_map>

bool WFPEngine::Open(DWORD auth) {
	if (m_hEngine)
		return true;
	m_LastError = FwpmEngineOpen(nullptr, auth, nullptr, nullptr, &m_hEngine);
	return m_LastError == ERROR_SUCCESS;
}

WFPEngine::~WFPEngine() {
	Close();
}

void WFPEngine::Close() {
	if (m_hEngine) {
		m_LastError = FwpmEngineClose(m_hEngine);
		m_hEngine = nullptr;
	}
}

DWORD WFPEngine::LastError() const {
	return m_LastError;
}

HANDLE WFPEngine::Handle() const {
	return m_hEngine;
}

WFPObject<FWPM_PROVIDER> WFPEngine::GetProviderByKey(GUID const& guid) const {
	FWPM_PROVIDER* provider = nullptr;
	m_LastError = FwpmProviderGetByKey(m_hEngine, &guid, &provider);
	return WFPObject(provider);
}

bool WFPEngine::AddProvider(FWPM_PROVIDER const* provider, PSECURITY_DESCRIPTOR sd) {
	m_LastError = ::FwpmProviderAdd(m_hEngine, provider, sd);
	return m_LastError == ERROR_SUCCESS;
}

bool WFPEngine::DeleteProvider(GUID const& key) {
	m_LastError = ::FwpmProviderDeleteByKey(m_hEngine, &key);
	return m_LastError == ERROR_SUCCESS;
}

WFPObject<FWPM_FILTER> WFPEngine::GetFilterByKey(GUID const& key) const {
	FWPM_FILTER* filter = nullptr;
	m_LastError = FwpmFilterGetByKey(m_hEngine, &key, &filter);
	return WFPObject(filter);
}

WFPObject<FWPM_FILTER>  WFPEngine::GetFilterById(UINT64 id, bool includeConditions) const {
	FWPM_FILTER* filter = nullptr;
	m_LastError = FwpmFilterGetById(m_hEngine, id, &filter);
	return WFPObject(filter);
}

bool WFPEngine::DeleteFilter(GUID const& key) {
	m_LastError = ::FwpmFilterDeleteByKey(m_hEngine, &key);
	return m_LastError == ERROR_SUCCESS;
}

bool WFPEngine::DeleteFilter(UINT64 id) {
	m_LastError = ::FwpmFilterDeleteById(m_hEngine, id);
	return m_LastError == ERROR_SUCCESS;
}

UINT64 WFPEngine::AddFilter(FWPM_FILTER const* filter, PSECURITY_DESCRIPTOR sd) {
	UINT64 id;
	m_LastError = ::FwpmFilterAdd(m_hEngine, filter, sd, &id);
	return m_LastError ? 0 : id;
}

WFPObject<FWPM_LAYER> WFPEngine::GetLayerByKey(GUID const& key) const {
	FWPM_LAYER* layer = nullptr;
	m_LastError = FwpmLayerGetByKey(m_hEngine, &key, &layer);
	return WFPObject(layer);
}

WFPObject<FWPM_LAYER> WFPEngine::GetLayerById(UINT16 id) const {
	FWPM_LAYER* layer;
	m_LastError = ::FwpmLayerGetById(m_hEngine, id, &layer);
	return WFPObject(layer);
}

bool WFPEngine::DeleteCallout(GUID const& key) {
	m_LastError = ::FwpmCalloutDeleteByKey(m_hEngine, &key);
	return m_LastError == ERROR_SUCCESS;
}

bool WFPEngine::DeleteCallout(UINT32 id) {
	m_LastError = ::FwpmCalloutDeleteById(m_hEngine, id);
	return m_LastError == ERROR_SUCCESS;
}

WFPObject<FWPM_SUBLAYER> WFPEngine::GetSublayerByKey(GUID const& key) const {
	FWPM_SUBLAYER* sublayer = nullptr;
	m_LastError = ::FwpmSubLayerGetByKey(m_hEngine, &key, &sublayer);
	return WFPObject(sublayer);
}

WFPObject<FWPM_CALLOUT> WFPEngine::GetCalloutByKey(GUID const& key) const {
	FWPM_CALLOUT* co = nullptr;
	FwpmCalloutGetByKey(m_hEngine, &key, &co);
	return WFPObject(co);
}

WFPObject<FWPM_CALLOUT> WFPEngine::GetCalloutById(UINT32 id) const {
	FWPM_CALLOUT* co = nullptr;
	FwpmCalloutGetById(m_hEngine, id, &co);
	return WFPObject(co);
}

uint32_t WFPEngine::GetFilterCount(GUID const& layer) const {
	HANDLE hEnum;
	m_LastError = FwpmFilterCreateEnumHandle(m_hEngine, nullptr, &hEnum);
	if (m_LastError)
		return 0;
	FWPM_FILTER** filters;
	UINT32 count;
	uint32_t total = 0;
	m_LastError = FwpmFilterEnum(m_hEngine, hEnum, 8192, &filters, &count);
	if (m_LastError == ERROR_SUCCESS) {
		for (UINT32 i = 0; i < count; i++) {
			auto filter = filters[i];
			if (filter->layerKey == layer)
				total++;
		}
		FwpmFreeMemory((void**)&filters);
	}
	m_LastError = FwpmFilterDestroyEnumHandle(m_hEngine, hEnum);
	return total;
}

uint32_t WFPEngine::GetCalloutCount(GUID const& layer) const {
	HANDLE hEnum;
	m_LastError = FwpmCalloutCreateEnumHandle(m_hEngine, nullptr, &hEnum);
	if (m_LastError)
		return 0;
	FWPM_CALLOUT** callouts;
	UINT32 count;
	uint32_t total = 0;
	m_LastError = FwpmCalloutEnum(m_hEngine, hEnum, 1024, &callouts, &count);
	if (m_LastError == ERROR_SUCCESS) {
		for (UINT32 i = 0; i < count; i++) {
			auto c = callouts[i];
			if (c->applicableLayer == layer)
				total++;
		}
		FwpmFreeMemory((void**)&callouts);
	}
	m_LastError = FwpmCalloutDestroyEnumHandle(m_hEngine, hEnum);
	return total;
}



