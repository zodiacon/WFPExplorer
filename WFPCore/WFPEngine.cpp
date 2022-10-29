#include "pch.h"
#include "WFPEngine.h"

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

std::vector<WFPSessionInfo> WFPEngine::EnumSessions() const {
	HANDLE hEnum;
	std::vector<WFPSessionInfo> info;
	m_LastError = FwpmSessionCreateEnumHandle(m_hEngine, nullptr, &hEnum);
	if (m_LastError != ERROR_SUCCESS)
		return info;

	UINT32 count;
	FWPM_SESSION** sessions;
	if ((m_LastError = FwpmSessionEnum(m_hEngine, hEnum, 128, &sessions, &count)) == ERROR_SUCCESS) {
		info.reserve(count);
		for (UINT32 i = 0; i < count; i++) {
			auto session = sessions[i];
			WFPSessionInfo si;
			si.Name = ParseMUIString(session->displayData.name);
			si.Desc = ParseMUIString(session->displayData.description);
			si.SessionKey = session->sessionKey;
			si.ProcessId = session->processId;
			si.UserName = session->username;
			si.Flags = session->flags;
			::CopySid(sizeof(si.Sid), (PSID)si.Sid, session->sid);
			si.KernelMode = session->kernelMode;
			info.emplace_back(std::move(si));
		}
		FwpmFreeMemory((void**)&sessions);
	}
	m_LastError = FwpmSessionDestroyEnumHandle(m_hEngine, hEnum);

	return info;
}

std::vector<WFPLayerInfo> WFPEngine::EnumLayers() const {
	HANDLE hEnum;
	std::vector<WFPLayerInfo> info;
	m_LastError = FwpmLayerCreateEnumHandle(m_hEngine, nullptr, &hEnum);
	if (m_LastError)
		return info;
	FWPM_LAYER** layers;
	UINT32 count;
	m_LastError = FwpmLayerEnum(m_hEngine, hEnum, 256, &layers, &count);
	if (m_LastError == ERROR_SUCCESS) {
		info.reserve(count);
		for (UINT32 i = 0; i < count; i++) {
			auto layer = layers[i];
			WFPLayerInfo li;
			li.Name = ParseMUIString(layer->displayData.name);
			li.Desc = ParseMUIString(layer->displayData.description);
			li.LayerKey = layer->layerKey;
			li.Flags = layer->flags;
			li.DefaultSubLayerKey = layer->defaultSubLayerKey;
			li.LayerId = layer->layerId;
			li.Fields.reserve(layer->numFields);
			for (UINT32 f = 0; f < layer->numFields; f++) {
				WFPFieldInfo fi;
				auto& field = layer->field[f];
				fi.DataType = (WFPDataType)field.dataType;
				fi.Type = (WFPFieldType)field.type;
				fi.FieldKey = *field.fieldKey;
				li.Fields.emplace_back(fi);
			}
			info.emplace_back(std::move(li));
		}
	}
	return info;
}

std::vector<WFPProviderInfo> WFPEngine::EnumProviders(bool includeData) const {
	HANDLE hEnum;
	std::vector<WFPProviderInfo> info;
	m_LastError = FwpmProviderCreateEnumHandle(m_hEngine, nullptr, &hEnum);
	if (m_LastError)
		return info;
	FWPM_PROVIDER** providers;
	UINT32 count;
	m_LastError = FwpmProviderEnum(m_hEngine, hEnum, 128, &providers, &count);
	if (m_LastError == ERROR_SUCCESS) {
		info.reserve(count);
		for (UINT32 i = 0; i < count; i++) {
			auto p = providers[i];
			info.emplace_back(std::move(InitProvider(p, includeData)));
		}
		FwpmFreeMemory((void**)&providers);
		m_LastError = FwpmProviderDestroyEnumHandle(m_hEngine, hEnum);
	}

	return info;
}

std::vector<WFPCalloutInfo> WFPEngine::EnumCallouts() const {
	HANDLE hEnum;
	std::vector<WFPCalloutInfo> info;
	m_LastError = FwpmCalloutCreateEnumHandle(m_hEngine, nullptr, &hEnum);
	if (m_LastError)
		return info;
	FWPM_CALLOUT** callouts;
	UINT32 count;
	m_LastError = FwpmCalloutEnum(m_hEngine, hEnum, 256, &callouts, &count);
	if (m_LastError == ERROR_SUCCESS) {
		info.reserve(count);
		for (UINT32 i = 0; i < count; i++) {
			auto c = callouts[i];
			WFPCalloutInfo ci;
			ci.Name = ParseMUIString(c->displayData.name);
			ci.Desc = ParseMUIString(c->displayData.description);
			ci.ProviderKey = c->providerKey ? *c->providerKey : GUID_NULL;
			ci.Flags = c->flags;
			ci.CalloutKey = c->calloutKey;
			ci.ProviderData.resize(c->providerData.size);
			memcpy(ci.ProviderData.data(), c->providerData.data, c->providerData.size);
			ci.CalloutId = c->calloutId;
			ci.ApplicableLayer = c->applicableLayer;
			info.emplace_back(std::move(ci));
		}
		FwpmFreeMemory((void**)&callouts);
		m_LastError = FwpmCalloutDestroyEnumHandle(m_hEngine, hEnum);
	}

	return info;
}

std::vector<WFPFilterInfo> WFPEngine::EnumFilters(bool includeConditions) const {
	HANDLE hEnum;
	std::vector<WFPFilterInfo> info;
	m_LastError = FwpmFilterCreateEnumHandle(m_hEngine, nullptr, &hEnum);
	if (m_LastError)
		return info;
	FWPM_FILTER** filters;
	UINT32 count;
	m_LastError = FwpmFilterEnum(m_hEngine, hEnum, 4096, &filters, &count);
	if (m_LastError == ERROR_SUCCESS) {
		info.reserve(count);
		for (UINT32 i = 0; i < count; i++) {
			auto filter = filters[i];
			WFPFilterInfo fi;
			fi.FilterKey = filter->filterKey;
			fi.FilterId = filter->filterId;
			fi.ProviderKey = filter->providerKey ? *filter->providerKey : GUID_NULL;
			fi.Name = ParseMUIString(filter->displayData.name);
			fi.Desc = ParseMUIString(filter->displayData.description);
			fi.ConditionCount = filter->numFilterConditions;
			fi.EffectiveWeight = *(WFPValue*)&filter->effectiveWeight;
			fi.LayerKey = filter->layerKey;
			fi.SubLayerKey = filter->subLayerKey;
			fi.Weight = *(WFPValue*)&filter->weight;
			if (includeConditions) {
				// TODO
			}
			info.emplace_back(std::move(fi));
		}
		FwpmFreeMemory((void**)&filters);
		m_LastError = FwpmFilterDestroyEnumHandle(m_hEngine, hEnum);
	}
	return info;
}

WFPProviderInfo WFPEngine::GetProviderByKey(GUID const& guid) const {
	FWPM_PROVIDER* provider;
	m_LastError = FwpmProviderGetByKey(m_hEngine, &guid, &provider);
	if (ERROR_SUCCESS != m_LastError)
		return {};
	auto p = InitProvider(provider);
	FwpmFreeMemory((void**)&provider);
	return p;
}

WFPProviderInfo WFPEngine::InitProvider(FWPM_PROVIDER* p, bool includeData) {
	WFPProviderInfo pi;
	pi.Name = ParseMUIString(p->displayData.name);
	pi.Desc = ParseMUIString(p->displayData.description);
	pi.ProviderKey = p->providerKey;
	pi.Flags = p->flags;
	pi.ProviderDataSize = p->providerData.size;
	if (includeData) {
		pi.ProviderData = std::make_unique<BYTE[]>(p->providerData.size);
		memcpy(pi.ProviderData.get(), p->providerData.data, p->providerData.size);
	}
	return pi;
}


std::wstring WFPEngine::ParseMUIString(PCWSTR input) {
	if (input == nullptr)
		return L"";

	if (*input && input[0] == L'@') {
		WCHAR result[256];
		if (::SHLoadIndirectString(input, result, _countof(result), nullptr) == S_OK)
			return result;
	}
	return input;
}
