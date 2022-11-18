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

std::vector<WFPConnectionInfo> WFPEngine::EnumConnections(bool includeData) {
	HANDLE hEnum;
	std::vector<WFPConnectionInfo> info;
	m_LastError = FwpmConnectionCreateEnumHandle(m_hEngine, nullptr, &hEnum);
	if (m_LastError)
		return info;
	FWPM_CONNECTION** connections;
	UINT32 count;
	m_LastError = FwpmConnectionEnum(m_hEngine, hEnum, 128, &connections, &count);
	if (m_LastError == ERROR_SUCCESS && count > 0) {
		info.reserve(count);
		for (UINT32 i = 0; i < count; i++) {
			auto p = connections[i];
			info.emplace_back(std::move(InitConnection(p, includeData)));
		}
		FwpmFreeMemory((void**)&connections);
		m_LastError = FwpmConnectionDestroyEnumHandle(m_hEngine, hEnum);
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

std::vector<WFPSystemPortByType> WFPEngine::EnumSystemPorts() {
	FWPM_SYSTEM_PORTS* ports;
	m_LastError = ::FwpmSystemPortsGet(m_hEngine, &ports);
	if (m_LastError)
		return {};

	std::vector<WFPSystemPortByType> sports;
	sports.reserve(ports->numTypes);
	for (UINT32 i = 0; i < ports->numTypes; i++) {
		WFPSystemPortByType p;
		auto& port = ports->types[i];
		p.Type = static_cast<WFPSystemPortType>(port.type);
		if (port.numPorts) {
			p.Ports.resize(port.numPorts);
			memcpy(p.Ports.data(), port.ports, sizeof(uint16_t) * port.numPorts);
		}
		sports.emplace_back(std::move(p));
	}
	::FwpmFreeMemory((void**)&ports);
	return sports;
}

std::optional<WFPProviderInfo> WFPEngine::GetProviderByKey(GUID const& guid) const {
	FWPM_PROVIDER* provider;
	m_LastError = FwpmProviderGetByKey(m_hEngine, &guid, &provider);
	if (ERROR_SUCCESS != m_LastError)
		return {};

	auto p = InitProvider(provider);
	FwpmFreeMemory((void**)&provider);
	return p;
}

std::optional<WFPFilterInfo> WFPEngine::GetFilterByKey(GUID const& key, bool includeConditions) const {
	FWPM_FILTER* filter;
	m_LastError = FwpmFilterGetByKey(m_hEngine, &key, &filter);
	if (m_LastError != ERROR_SUCCESS)
		return {};

	auto info = InitFilter(filter, includeConditions);
	FwpmFreeMemory((void**)&filter);
	return info;
}

std::optional<WFPFilterInfo> WFPEngine::GetFilterById(UINT64 id, bool includeConditions) const {
	FWPM_FILTER* filter;
	m_LastError = FwpmFilterGetById(m_hEngine, id, &filter);
	if (m_LastError != ERROR_SUCCESS)
		return {};

	auto info = InitFilter(filter, includeConditions);
	FwpmFreeMemory((void**)&filter);
	return info;
}

std::optional<WFPLayerInfo> WFPEngine::GetLayerByKey(GUID const& key) const {
	FWPM_LAYER* layer;
	m_LastError = FwpmLayerGetByKey(m_hEngine, &key, &layer);
	if (m_LastError != ERROR_SUCCESS)
		return {};

	auto info = InitLayer(layer, true);
	FwpmFreeMemory((void**)&layer);
	return info;
}

std::optional<WFPSubLayerInfo> WFPEngine::GetSublayerByKey(GUID const& key) const {
	FWPM_SUBLAYER* sublayer;
	FwpmSubLayerGetByKey(m_hEngine, &key, &sublayer);
	auto info = InitSubLayer(sublayer);
	FwpmFreeMemory((void**)&sublayer);
	return info;
}

WFPProviderInfo WFPEngine::InitProvider(FWPM_PROVIDER* p, bool includeData) {
	WFPProviderInfo pi;
	pi.Name = ParseMUIString(p->displayData.name);
	pi.Desc = ParseMUIString(p->displayData.description);
	pi.ProviderKey = p->providerKey;
	pi.Flags = static_cast<WFPProviderFlags>(p->flags);
	pi.ServiceName = p->serviceName ? p->serviceName : L"";
	pi.ProviderDataSize = p->providerData.size;
	if (includeData) {
		pi.ProviderData = std::make_unique<BYTE[]>(p->providerData.size);
		memcpy(pi.ProviderData.get(), p->providerData.data, p->providerData.size);
	}
	return pi;
}

WFPProviderContextInfo WFPEngine::InitProviderContext(FWPM_PROVIDER_CONTEXT* p, bool includeData) {
	WFPProviderContextInfo pi;
	pi.Name = ParseMUIString(p->displayData.name);
	pi.Desc = ParseMUIString(p->displayData.description);
	pi.ProviderContextKey = p->providerContextKey;
	pi.ProviderContextId = p->providerContextId;
	pi.Flags = static_cast<WFPProviderContextFlags>(p->flags);
	pi.ProviderDataSize = p->providerData.size;
	pi.ProviderKey = p->providerKey ? *p->providerKey : GUID_NULL;
	pi.Type = static_cast<WFPProviderContextType>(p->type);
	if (includeData) {
		pi.ProviderData.resize(p->providerData.size);
		memcpy(pi.ProviderData.data(), p->providerData.data, p->providerData.size);
	}
	return pi;
}

WFPConnectionInfo WFPEngine::InitConnection(FWPM_CONNECTION* p, bool includeData) {
	WFPConnectionInfo ci;
	ci.ConnectionId = p->connectionId;
	ci.ProviderKey = p->providerKey ? *p->providerKey : GUID_NULL;
	ci.IpVersion = static_cast<WFPIPVersion>(p->ipVersion);
	if (ci.IpVersion == WFPIPVersion::V4) {
		ci.LocalV4Address = p->localV4Address;
		ci.RemoteV4Address = p->remoteV4Address;
	}
	else {
		memcpy(ci.LocalV6Address, p->localV6Address, sizeof(ci.LocalV6Address));
		memcpy(ci.RemoteV6Address, p->remoteV6Address, sizeof(ci.RemoteV6Address));
	}
	ci.IpSecTrafficModeType = static_cast<WFPIPSecTrafficType>(p->ipsecTrafficModeType);
	ci.KeyModuleType = static_cast<WFPIkeExtKeyModuleType>(p->keyModuleType);

	if (includeData) {
	}
	return ci;
}

std::wstring WFPEngine::PoorParseMUIString(std::wstring const& path) {
	static std::unordered_map<std::wstring, std::wstring> cache;

	if (path[0] != L'@')
		return path;

	if (auto it = cache.find(path); it != cache.end())
		return it->second;

	auto comma = path.find(L',', 1);
	if (comma == std::wstring::npos)
		return path;

	auto dllname = path.substr(1, comma - 1);
	auto hDll = ::LoadLibraryEx(dllname.c_str(), nullptr, LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE);
	if (!hDll)
		return path;

	CString result;
	auto id = _wtoi(path.substr(comma + 1).c_str());
	if (id < 0)
		id = -id;
	result.LoadStringW(hDll, id);
	::FreeLibrary(hDll);
	if (!result.IsEmpty()) {
		cache.insert({ path, (PCWSTR)result });
		return (PCWSTR)result;
	}
	if (dllname.substr(dllname.rfind(L'.')) != L".mui")
		return PoorParseMUIString(L"@c:\\Windows\\System32\\en-US\\" + dllname + L".mui" + path.substr(comma));
	return path;
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

WFPValue& WFPValue::Init(FWP_VALUE const& value) {
	Type = static_cast<WFPDataType>(value.type);

	switch (Type) {
		case WFPDataType::INT64: int64 = *value.int64; break;
		case WFPDataType::UINT64: uint64 = *value.uint64; break;
		case WFPDataType::DOUBLE: double64 = *value.double64; break;
		default:
			memcpy(this, &value, sizeof(value));
	}
	return *this;
}

std::vector<WFPNetEventInfo> WFPEngine::EnumNetEvents() {
	std::vector<WFPNetEventInfo> events;
	HANDLE hEnum;
	m_LastError = FwpmNetEventCreateEnumHandle(m_hEngine, nullptr, &hEnum);
	if (m_LastError != ERROR_SUCCESS)
		return {};

	FWPM_NET_EVENT** pEvents;
	UINT32 count;
	m_LastError = FwpmNetEventEnum(m_hEngine, hEnum, 128, &pEvents, &count);
	if (ERROR_SUCCESS == m_LastError) {
	}
	return events;
}

std::vector<WFPProviderContextInfo> WFPEngine::EnumProviderContexts(bool includeData) const {
	HANDLE hEnum;
	std::vector<WFPProviderContextInfo> info;
	m_LastError = FwpmProviderContextCreateEnumHandle(m_hEngine, nullptr, &hEnum);
	if (m_LastError)
		return info;
	FWPM_PROVIDER_CONTEXT** contexts;
	UINT32 count;
	m_LastError = FwpmProviderContextEnum(m_hEngine, hEnum, 128, &contexts, &count);
	if (m_LastError == ERROR_SUCCESS && count > 0) {
		info.reserve(count);
		for (UINT32 i = 0; i < count; i++) {
			auto p = contexts[i];
			info.emplace_back(std::move(InitProviderContext(p, includeData)));
		}
		FwpmFreeMemory((void**)&contexts);
		m_LastError = FwpmProviderContextDestroyEnumHandle(m_hEngine, hEnum);
	}

	return info;
}

std::optional<WFPCalloutInfo> WFPEngine::GetCalloutByKey(GUID const& key) const {
	FWPM_CALLOUT* co;
	FwpmCalloutGetByKey(m_hEngine, &key, &co);
	auto info = InitCallout(co);
	FwpmFreeMemory((void**)&co);
	return info;
}

WFPConditionValue& WFPConditionValue::Init(FWP_CONDITION_VALUE const& value) {
	Type = static_cast<WFPDataType>(value.type);
	switch (Type) {
		case WFPDataType::INT64: int64 = *value.int64; break;
		case WFPDataType::UINT64: uint64 = *value.uint64; break;
		case WFPDataType::DOUBLE: double64 = *value.double64; break;
		default:
			memcpy(this, &value, sizeof(value));
	}

	return *this;
}
