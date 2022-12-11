#pragma once

#include <vector>
#include <atlsecurity.h>
#include <memory>
#include <type_traits>
#include <optional>

enum class WFPProviderContextFlags {
	None,
	Persistent = FWPM_PROVIDER_CONTEXT_FLAG_PERSISTENT,
	DownLevel = FWPM_PROVIDER_CONTEXT_FLAG_DOWNLEVEL,
};
DEFINE_ENUM_FLAG_OPERATORS(WFPProviderContextFlags);

enum class WFPProviderContextType {
	IPSecKeying						= FWPM_IPSEC_KEYING_CONTEXT,
	IPSecIkeQuickModeTransport		= FWPM_IPSEC_IKE_QM_TRANSPORT_CONTEXT,
	IPSecIkeQuickModeTunnel			= FWPM_IPSEC_IKE_QM_TUNNEL_CONTEXT,
	IPSecAuthIPQuickModeTransport	= FWPM_IPSEC_AUTHIP_QM_TRANSPORT_CONTEXT,
	IPSecAuthIPQuickModeTunnel		= FWPM_IPSEC_AUTHIP_QM_TUNNEL_CONTEXT,
	IPSecIkeMainMode				= FWPM_IPSEC_IKE_MM_CONTEXT,
	IPSecAuthIPMainMode				= FWPM_IPSEC_AUTHIP_MM_CONTEXT,
	ClassifyOptions					= FWPM_CLASSIFY_OPTIONS_CONTEXT,
	General							= FWPM_GENERAL_CONTEXT,
	IPSecIkeV2QuickModeTunnel		= FWPM_IPSEC_IKEV2_QM_TUNNEL_CONTEXT,
	IPSecIKeV2MainMode				= FWPM_IPSEC_IKEV2_MM_CONTEXT,
	IPSecDosProtection				= FWPM_IPSEC_DOSP_CONTEXT,
	IPSecIkeV2QuickModeTransport	= FWPM_IPSEC_IKEV2_QM_TRANSPORT_CONTEXT,
	_Max							= FWPM_PROVIDER_CONTEXT_TYPE_MAX
};

struct WFPProviderContextInfo {
	GUID ProviderContextKey;
	std::wstring Name;
	std::wstring Desc;
	WFPProviderContextFlags Flags;
	GUID ProviderKey;
	uint32_t ProviderDataSize;
	std::vector<BYTE> ProviderData;
	UINT64 ProviderContextId;
	WFPProviderContextType Type;
};

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

	//
	// enumerations
	//

	uint32_t GetFilterCount(GUID const& layer = GUID_NULL) const;
	uint32_t GetCalloutCount(GUID const& layer = GUID_NULL) const;

	std::vector<WFPProviderContextInfo> EnumProviderContexts(bool includeData = false) const;

	//
	// providers API
	//
	WFPObject<FWPM_PROVIDER> GetProviderByKey(GUID const& key) const;

	//
	// Filters API
	//
	WFPObject<FWPM_FILTER> GetFilterByKey(GUID const& key) const;
	WFPObject<FWPM_FILTER> GetFilterById(UINT64 id, bool full = true) const;
	bool DeleteFilter(GUID const& key);
	bool DeleteFilter(UINT64 id);

	//
	// layer API
	//
	WFPObject<FWPM_LAYER> GetLayerByKey(GUID const& key) const;
	WFPObject<FWPM_LAYER> GetLayerById(UINT16 id) const;

	//
	// sublayer API
	//
	WFPObject<FWPM_SUBLAYER> GetSublayerByKey(GUID const& key) const;
	WFPObject<FWPM_SUBLAYER> GetSublayerById(UINT16 id) const;


	WFPObject<FWPM_CALLOUT> GetCalloutByKey(GUID const& key) const;

private:
	//
	// helpers
	//
	static std::wstring ParseMUIString(PCWSTR input);
	static WFPProviderContextInfo InitProviderContext(FWPM_PROVIDER_CONTEXT* p, bool full = false);

	HANDLE m_hEngine{ nullptr };
	mutable DWORD m_LastError{ 0 };
};

