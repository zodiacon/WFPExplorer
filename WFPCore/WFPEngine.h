#pragma once

#include <vector>

template<typename T>
struct WFPObject {
	explicit WFPObject(T* p) : Data(p) {}
	~WFPObject() {
		if(Data)
			::FwpmFreeMemory((void**)&Data);
	}

	T* operator->() const {
		return Data;
	}

	T* operator*() {
		return Data;
	}

	operator bool() const {
		return Data != nullptr;
	}

private:
	T* Data;
};

class WFPEngine {
public:
	bool Open(DWORD auth = RPC_C_AUTHN_WINNT);
	~WFPEngine();
	void Close();
	DWORD LastError() const;
	HANDLE Handle() const;

	uint32_t GetFilterCount(GUID const& layer = GUID_NULL) const;
	uint32_t GetCalloutCount(GUID const& layer = GUID_NULL) const;

	//
	// providers API
	//
	WFPObject<FWPM_PROVIDER> GetProviderByKey(GUID const& key) const;
	bool AddProvider(FWPM_PROVIDER const* provider, PSECURITY_DESCRIPTOR sd = nullptr);
	bool DeleteProvider(GUID const& key);

	//
	// Filters API
	//
	WFPObject<FWPM_FILTER> GetFilterByKey(GUID const& key) const;
	WFPObject<FWPM_FILTER> GetFilterById(UINT64 id, bool full = true) const;
	bool DeleteFilter(GUID const& key);
	bool DeleteFilter(UINT64 id);
	UINT64 AddFilter(FWPM_FILTER const* filter, PSECURITY_DESCRIPTOR sd = nullptr);

	//
	// layer API
	//
	WFPObject<FWPM_LAYER> GetLayerByKey(GUID const& key) const;
	WFPObject<FWPM_LAYER> GetLayerById(UINT16 id) const;

	//
	// Callout API
	//
	bool DeleteCallout(GUID const& key);
	bool DeleteCallout(UINT32 id);

	//
	// sublayer API
	//
	WFPObject<FWPM_SUBLAYER> GetSublayerByKey(GUID const& key) const;
	WFPObject<FWPM_CALLOUT> GetCalloutByKey(GUID const& key) const;
	WFPObject<FWPM_CALLOUT> GetCalloutById(UINT32 id) const;

private:
	HANDLE m_hEngine{ nullptr };
	mutable DWORD m_LastError{ 0 };
};

