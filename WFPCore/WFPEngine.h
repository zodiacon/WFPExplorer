#pragma once

#include <vector>
#include <atlsecurity.h>
#include <memory>
#include <type_traits>
#include <optional>

enum class WFPSessionFlags {
	None,
	Dynamic = FWPM_SESSION_FLAG_DYNAMIC,
	Reserved = FWPM_SESSION_FLAG_RESERVED,
};
DEFINE_ENUM_FLAG_OPERATORS(WFPSessionFlags);

enum class WFPIPVersion {
	V4 = 0,
	V6,
	NONE,
};

struct WFPSessionInfo {
	GUID SessionKey;
	std::wstring Name;
	std::wstring Desc;
	WFPSessionFlags Flags;
	UINT32 TxWaitTimeoutInMSec;
	DWORD ProcessId;
	std::vector<BYTE> Sid;
	std::wstring UserName;
	bool KernelMode;
};

enum class WFPProviderContextFlags {
	None,
	Persistent = FWPM_PROVIDER_CONTEXT_FLAG_PERSISTENT,
	DownLevel = FWPM_PROVIDER_CONTEXT_FLAG_DOWNLEVEL,
};
DEFINE_ENUM_FLAG_OPERATORS(WFPProviderContextFlags);

enum class WFPFieldType {
	RawData,
	IpAddress,
	Flags,
};

enum class WFPDataType {
	EMPTY,
	UINT8,
	UINT16,
	UINT32,
	UINT64,
	INT8,
	INT16,
	INT32,
	INT64,
	FLOAT,
	DOUBLE,
	BYTE_ARRAY16_TYPE,
	BYTE_BLOB_TYPE,
	SID,
	SECURITY_DESCRIPTOR_TYPE,
	TOKEN_INFORMATION_TYPE,
	TOKEN_ACCESS_INFORMATION_TYPE,
	UNICODE_STRING_TYPE,
	BYTE_ARRAY6_TYPE,
	BITMAP_INDEX_TYPE,
	BITMAP_ARRAY64_TYPE,
	SINGLE_DATA_TYPE_MAX = 0xff,
	V4_ADDR_MASK,
	V6_ADDR_MASK,
	RANGE_TYPE,
};

struct WFPRange;

struct WFPValue {
	WFPValue();
	WFPValue(WFPValue const& other);
	WFPValue& operator=(WFPValue const& other);
	WFPValue(WFPValue&& other) noexcept;
	WFPValue& operator=(WFPValue&& other) noexcept;

	WFPDataType Type;
	union {
		UINT8 uint8;
		UINT16 uint16;
		UINT32 uint32;
		UINT64 uint64;
		INT8 int8;
		INT16 int16;
		INT32 int32;
		INT64 int64;
		float float32;
		double double64;
		BYTE* byteArray16;
		FWP_BYTE_BLOB* byteBlob;
		SID* sid;
		FWP_BYTE_BLOB* sd;
		FWP_TOKEN_INFORMATION* tokenInformation;
		FWP_BYTE_BLOB* tokenAccessInformation;
		LPWSTR unicodeString;
		BYTE byteArray6[6];
		FWP_V4_ADDR_AND_MASK* v4AddrMask;
		FWP_V6_ADDR_AND_MASK* v6AddrMask;
		WFPRange* rangeValue;
	};
	void Free();
	~WFPValue();

	WFPValue& Init(WFPValue const& value);

	template<typename T>
	WFPValue& Init(T const& value) {
		Type = static_cast<WFPDataType>(value.type);

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

			case WFPDataType::V4_ADDR_MASK:
				if constexpr (std::is_base_of_v<T, FWP_CONDITION_VALUE>) {
					v4AddrMask = new FWP_V4_ADDR_AND_MASK;
					*v4AddrMask = *value.v4AddrMask;
				}
				break;

			case WFPDataType::V6_ADDR_MASK:
				if constexpr (std::is_base_of_v<T, FWP_CONDITION_VALUE>) {
					v6AddrMask = new FWP_V6_ADDR_AND_MASK;
					*v6AddrMask = *value.v6AddrMask;
				}
				break;

			case WFPDataType::RANGE_TYPE:
				if constexpr (std::is_base_of_v<T, FWP_CONDITION_VALUE>) {
					rangeValue = new WFPRange(value.rangeValue->valueLow, value.rangeValue->valueHigh);
				}
				break;

			case WFPDataType::INT64: int64 = *value.int64; break;
			case WFPDataType::UINT64: uint64 = *value.uint64; break;
			case WFPDataType::DOUBLE: double64 = *value.double64; break;
			default:
				memcpy(this, &value, sizeof(value));
		}
		return *this;
	}

};

struct WFPRange {
	WFPValue Low;
	WFPValue High;

	WFPRange() = default;
	WFPRange(WFPRange const&) = default;
	WFPRange(WFPRange&&) = default;
	WFPRange& operator=(WFPRange const&) = default;
	WFPRange& operator=(WFPRange&&) = default;

	WFPRange(WFPValue low, WFPValue high) noexcept;
	template<typename T>
	WFPRange(T const& low, T const& high) noexcept {
		Low.Init(low);
		High.Init(high);
	}
};

enum class WFPLayerFlags {
	None,
	Kernel = FWPM_LAYER_FLAG_KERNEL,
	BuiltIn = FWPM_LAYER_FLAG_BUILTIN,
	ClassifyMostly = FWPM_LAYER_FLAG_CLASSIFY_MOSTLY,
	Buffered = FWPM_LAYER_FLAG_BUFFERED,
};
DEFINE_ENUM_FLAG_OPERATORS(WFPLayerFlags);

enum class WFPFilterFlags {
	None = FWPM_FILTER_FLAG_NONE,
	Persistent = FWPM_FILTER_FLAG_PERSISTENT,
	BootTime = FWPM_FILTER_FLAG_BOOTTIME,
	HasProviderContext = FWPM_FILTER_FLAG_HAS_PROVIDER_CONTEXT,
	ClearActionRight = FWPM_FILTER_FLAG_CLEAR_ACTION_RIGHT,
	PermitIfCalloutUnregistered = FWPM_FILTER_FLAG_PERMIT_IF_CALLOUT_UNREGISTERED,
	Disabled = FWPM_FILTER_FLAG_DISABLED,
	Indexed = FWPM_FILTER_FLAG_INDEXED,
	HasSecurityRealmProviderContext = FWPM_FILTER_FLAG_HAS_SECURITY_REALM_PROVIDER_CONTEXT,
	SystemOsOnly = FWPM_FILTER_FLAG_SYSTEMOS_ONLY,
	GameOsOnly = FWPM_FILTER_FLAG_GAMEOS_ONLY,
	SilentMode = FWPM_FILTER_FLAG_SILENT_MODE,
	IPSecNoAcquireInitiate = FWPM_FILTER_FLAG_IPSEC_NO_ACQUIRE_INITIATE,
	Reserved0 = FWPM_FILTER_FLAG_RESERVED0,
	Reserved1 = FWPM_FILTER_FLAG_RESERVED1,
};
DEFINE_ENUM_FLAG_OPERATORS(WFPFilterFlags);

struct WFPFieldInfo {
	GUID FieldKey;
	WFPFieldType Type;
	WFPDataType DataType;
};

struct WFPLayerInfo {
	GUID LayerKey;
	std::wstring Name;
	std::wstring Desc;
	WFPLayerFlags Flags;
	UINT32 NumFields;
	std::vector<WFPFieldInfo> Fields;
	GUID DefaultSubLayerKey;
	UINT16 LayerId;
};

enum class WFPSubLayerFlags {
	None = 0,
	Persistent = FWPM_SUBLAYER_FLAG_PERSISTENT,
};
DEFINE_ENUM_FLAG_OPERATORS(WFPSubLayerFlags);

struct WFPSubLayerInfo {
	GUID SubLayerKey;
	std::wstring Name;
	std::wstring Desc;
	WFPSubLayerFlags Flags;
	GUID ProviderKey;
	std::vector<BYTE> ProviderData;
	uint32_t ProviderDataSize;
	UINT16 Weight;
};

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
	//union {
	//    IPSEC_KEYING_POLICY1* keyingPolicy;
	//    IPSEC_TRANSPORT_POLICY2* ikeQmTransportPolicy;
	//    IPSEC_TUNNEL_POLICY3* ikeQmTunnelPolicy;
	//    IPSEC_TRANSPORT_POLICY2* authipQmTransportPolicy;
	//    IPSEC_TUNNEL_POLICY3* authipQmTunnelPolicy;
	//    IKEEXT_POLICY2* ikeMmPolicy;
	//    IKEEXT_POLICY2* authIpMmPolicy;
	//    FWP_BYTE_BLOB* dataBuffer;
	//    FWPM_CLASSIFY_OPTIONS0* classifyOptions;
	//    IPSEC_TUNNEL_POLICY3* ikeV2QmTunnelPolicy;
	//    IPSEC_TRANSPORT_POLICY2* ikeV2QmTransportPolicy;
	//    IKEEXT_POLICY2* ikeV2MmPolicy;
	//    IPSEC_DOSP_OPTIONS0* idpOptions;
	//    FWPM_NETWORK_CONNECTION_POLICY_SETTINGS0* networkConnectionPolicy;
	//};
};

enum class WFPMatchType {
	Equal,
	Greater,
	Less,
	GreaterOrEqual,
	LessOrEqual,
	Range,
	FlagsAllSet,
	FlagsAnySet,
	FlagsNoneSet,
	EqualCaseInsensitive,
	NotEqual,
	Prefix,
	NotPrefix,
};

struct WFPFilterCondition {
	GUID FieldKey;
	WFPMatchType MatchType;
	WFPValue Value;
};

enum class WFPActionType {
	Block				= 0x00000001 | FWP_ACTION_FLAG_TERMINATING,
	Permit				= 0x00000002 | FWP_ACTION_FLAG_TERMINATING,
	CalloutTerminating	= 0x00000003 | FWP_ACTION_FLAG_CALLOUT | FWP_ACTION_FLAG_TERMINATING,
	CalloutInspection	= 0x00000004 | FWP_ACTION_FLAG_CALLOUT | FWP_ACTION_FLAG_NON_TERMINATING,
	CalloutUnknown		= 0x00000005 | FWP_ACTION_FLAG_CALLOUT,
	Continue			= 0x00000006 | FWP_ACTION_FLAG_NON_TERMINATING,
	None				= 0x00000007,
	NoneNoMatch			= 0x00000008,
};

struct WFPFilterAction {
	WFPActionType Type;
	union {
		GUID FilterType;
		GUID CalloutKey;
	};
};

static_assert(sizeof(WFPFilterAction) == sizeof(FWPM_ACTION));

struct WFPFilterInfo {
	GUID FilterKey;
	std::wstring Name;
	std::wstring Desc;
	WFPFilterFlags Flags;
	GUID ProviderKey;
	std::vector<BYTE> ProviderData;
	uint32_t ProviderDataSize;
	GUID LayerKey;
	GUID SubLayerKey;
	WFPValue Weight;
	UINT32 ConditionCount;
	std::vector<WFPFilterCondition> Conditions;
	WFPFilterAction Action;
	union {
		UINT64 RawContext;
		GUID ProviderContextKey;
	};
	UINT64 FilterId;
	WFPValue EffectiveWeight;
};

enum class WFPCalloutFlags {
	None = 0,
	Persistent = FWPM_CALLOUT_FLAG_PERSISTENT,
	UsesProviderContext = FWPM_CALLOUT_FLAG_USES_PROVIDER_CONTEXT,
	Registered = FWPM_CALLOUT_FLAG_REGISTERED,
};
DEFINE_ENUM_FLAG_OPERATORS(WFPCalloutFlags);

struct WFPCalloutInfo {
	GUID CalloutKey;
	std::wstring Name;
	std::wstring Desc;
	WFPCalloutFlags Flags;
	GUID ProviderKey;
	std::vector<BYTE> ProviderData;
	uint32_t ProviderDataSize;
	GUID ApplicableLayer;
	UINT32 CalloutId;
};

enum class WFPNetEventType {
	IkeextMmFailure = 0,
	IkeextQmFailure,
	IkeextEmFailure,
	ClassifyDrop,
	IpsecKernelDrop,
	IpsecDospDrop,
	ClassifyAllow,
	CapabilityDrop,
	CapabilityAllow,
	ClassifyDropMac,
	LpmPacketArrival,
};

struct WFPNetEventHeader0 {
	FILETIME TimeStamp;
	UINT32 Flags;
	WFPIPVersion IpVersion;
	UINT8 IpProtocol;
	union {
		uint32_t LocalAddrV4;
		uint8_t LocalAddrV6[16];
	};
	union {
		uint32_t RemoteAddrV4;
		uint8_t RemoteAddrV6[16];
	};
	uint16_t LocalPort;
	uint16_t RemotePort;
	uint32_t ScopeId;
	std::vector<uint8_t> AppId;
	std::vector<uint8_t> UserId;
};

enum class WFPAddressFamily {
	Inet = FWP_IP_VERSION_V4,
	Inet6 = FWP_IP_VERSION_V6,
	Ether = FWP_IP_VERSION_NONE,
	None = (FWP_AF_ETHER + 1)
};

enum class WFPIPSecTrafficType {
	Transport = 0,
	Tunnel,
};

enum class WFPIkeExtKeyModuleType {
	Ike = 0,
	AuthIP,
	IkeV2,
};

enum class WFPIkeExtCipherType {
	DES = 0,
	_3DES,
	AES_128,
	AES_192,
	AES_256,
	AES_GCM_128_16ICV,
	AES_GCM_256_16ICV,
};

struct WFPIkeExtCipherAlgorithm {
	WFPIkeExtCipherType Algorithm;
	uint32_t KeyLen;
	uint32_t Rounds;
};;

enum class WFPIkeExtIntegrityType {
	MD5 = 0,
	SHA1,
	SHA_256,
	SHA_384,
};

struct WFPIkeExtIntegrityAlgorithm {
	WFPIkeExtIntegrityType AlgoIdentifier;
};

enum class WFPIkeExtDhGroup {
	GROUP_NONE = 0,
	GROUP_1,
	GROUP_2,
	GROUP_14,
	GROUP_2048,
	ECP_256,
	ECP_384,
	GROUP_24,
};

struct WFPIkeExtProposal {
	WFPIkeExtCipherAlgorithm CipherAlgorithm;
	WFPIkeExtIntegrityAlgorithm IntegrityAlgorithm;
	uint32_t MaxLifetimeSeconds;
	WFPIkeExtDhGroup dhGroup;
	uint32_t QuickModeLimit;
};

enum class WFPIkeExtAuthenticationMethodType {
	PRESHARED_KEY = 0,
	CERTIFICATE,
	KERBEROS,
	ANONYMOUS,
	SSL,
	NTLM_V2,
	IPV6_CGA,
	CERTIFICATE_ECDSA_P256,
	CERTIFICATE_ECDSA_P384,
	SSL_ECDSA_P256,
	SSL_ECDSA_P384,
	EAP,
	RESERVED,
};

enum WFPIkeExtImpersonationMethodType {
	NONE = 0,
	SOCKET_PRINCIPAL,
};

struct WFPIkeExtCredential {
	WFPIkeExtAuthenticationMethodType AuthenticationMethodType;
	WFPIkeExtImpersonationMethodType ImpersonationType;
	union {
		IKEEXT_PRESHARED_KEY_AUTHENTICATION1* presharedKey;
		IKEEXT_CERTIFICATE_CREDENTIAL1* certificate;
		IKEEXT_NAME_CREDENTIAL0* name;
	};
};

struct WFPConnectionInfo {
	uint64_t ConnectionId;
	WFPIPVersion IpVersion;
	union {
		uint32_t LocalV4Address;
		uint8_t LocalV6Address[16];
	};
	union {
		uint32_t RemoteV4Address;
		uint8_t RemoteV6Address[16];
	};
	GUID ProviderKey;
	WFPIPSecTrafficType IpSecTrafficModeType;
	WFPIkeExtKeyModuleType KeyModuleType;
	WFPIkeExtProposal MmCrypto;
	WFPIkeExtCredential MmPeer;
	WFPIkeExtCredential EmPeer;
	uint64_t BytesTransferredIn;
	uint64_t BytesTransferredOut;
	uint64_t BytesTransferredTotal;
	int64_t StartSysTime;
};

enum class WFPSystemPortType {
	RPC_EPMAP = 0,
	Teredo,
	IPHTTPS_In,
	IPHTTPS_Out,
};

struct WFPSystemPortByType {
	WFPSystemPortType Type;
	std::vector<uint16_t> Ports;
};

struct WFPAleEndpointProperties {
	uint64_t EndpointId;
	WFPIPVersion IpVersion;
	union {
		uint32_t LocalV4Address;
		uint8_t LocalV6Address[16];
	};
	union {
		uint32_t RemoteV4Address;
		uint8_t RemoteV6Address[16];
	};
	uint8_t IpProtocol;
	uint16_t LocalPort;
	uint16_t RemotePort;
	uint64_t LocalTokenModifiedId;
	uint64_t MmmSaId;
	uint64_t QmSaId;
	uint32_t IpsecStatus;
	uint32_t Flags;
	uint32_t AppIdSize;
	std::vector<BYTE> AppId;
};

template<typename T>
struct WFPObject {
	WFPObject(T* p) : Data(p) {}
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

	std::vector<WFPConnectionInfo> EnumConnections(bool includeData = false);
	std::vector<WFPSystemPortByType> EnumSystemPorts();

	template<typename TFilter = WFPFilterInfo> requires std::is_base_of_v<WFPFilterInfo, TFilter>
	std::vector<TFilter> EnumFilters(GUID const& layer, bool full = false) const {
		if (layer == GUID_NULL)
			return EnumFilters<TFilter>(full);
		if (layer == GUID_NULL)
			return EnumFilters<TFilter>(full);

		HANDLE hEnum;
		std::vector<TFilter> info;
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
				if (filter->layerKey == layer) {
					auto fi = InitFilter<TFilter>(filter, full);
					info.emplace_back(std::move(fi));
				}
			}
			FwpmFreeMemory((void**)&filters);
		}
		m_LastError = FwpmFilterDestroyEnumHandle(m_hEngine, hEnum);
		return info;
	}

	template<typename TFilter = WFPFilterInfo> requires std::is_base_of_v<WFPFilterInfo, TFilter>
	std::vector<TFilter> EnumFilters(bool full = false) const {
		HANDLE hEnum;
		std::vector<TFilter> info;
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
				auto fi = InitFilter<TFilter>(filter, full);
				info.emplace_back(std::move(fi));
			}
			FwpmFreeMemory((void**)&filters);
		}
		m_LastError = FwpmFilterDestroyEnumHandle(m_hEngine, hEnum);
		return info;
	}

	template<typename T = WFPLayerInfo> requires std::is_base_of_v<WFPLayerInfo, T>
	std::vector<T> EnumLayers(bool includeFields = false) const {
		HANDLE hEnum;
		std::vector<T> info;
		m_LastError = FwpmLayerCreateEnumHandle(m_hEngine, nullptr, &hEnum);
		if (m_LastError)
			return info;
		FWPM_LAYER** layers;
		UINT32 count;
		m_LastError = FwpmLayerEnum(m_hEngine, hEnum, 512, &layers, &count);
		if (m_LastError == ERROR_SUCCESS) {
			info.reserve(count);
			for (UINT32 i = 0; i < count; i++) {
				auto layer = layers[i];
				auto li = InitLayer(layer, includeFields);
				info.emplace_back(std::move(li));
			}
			FwpmFreeMemory((void**)&layers);
		}
		m_LastError = FwpmLayerDestroyEnumHandle(m_hEngine, hEnum);
		return info;
	}

	template<typename T = WFPSubLayerInfo> requires std::is_base_of_v<WFPSubLayerInfo, T>
	std::vector<T> EnumSubLayers() const {
		HANDLE hEnum;
		std::vector<T> info;
		m_LastError = FwpmSubLayerCreateEnumHandle(m_hEngine, nullptr, &hEnum);
		if (m_LastError)
			return info;
		FWPM_SUBLAYER** layers;
		UINT32 count;
		m_LastError = FwpmSubLayerEnum(m_hEngine, hEnum, 256, &layers, &count);
		if (m_LastError == ERROR_SUCCESS) {
			info.reserve(count);
			for (UINT32 i = 0; i < count; i++) {
				auto layer = layers[i];
				auto li = InitSubLayer(layer);
				info.emplace_back(std::move(li));
			}
			FwpmFreeMemory((void**)&layers);
		}
		m_LastError = FwpmSubLayerDestroyEnumHandle(m_hEngine, hEnum);
		return info;

	}

	std::vector<WFPProviderContextInfo> EnumProviderContexts(bool includeData = false) const;

	//
	// providers API
	//
	WFPObject<FWPM_PROVIDER> GetProviderByKey(GUID const& key) const;

	//
	// Filters API
	//
	std::optional<WFPFilterInfo> GetFilterByKey(GUID const& key, bool full = true) const;
	std::optional<WFPFilterInfo> GetFilterById(UINT64 id, bool full = true) const;
	bool DeleteFilter(GUID const& key);
	bool DeleteFilter(UINT64 id);

	//
	// layer API
	//
	WFPObject<FWPM_LAYER> GetLayerByKey(GUID const& key) const;
	std::optional<WFPLayerInfo> GetLayerById(UINT16 id) const;

	//
	// sublayer API
	//
	std::optional<WFPSubLayerInfo> GetSublayerByKey(GUID const& key) const;
	std::optional<WFPSubLayerInfo> GetSublayerById(UINT16 id) const;


	WFPObject<FWPM_CALLOUT> GetCalloutByKey(GUID const& key) const;

private:
	//
	// helpers
	//
	static std::wstring ParseMUIString(PCWSTR input);
	static WFPConnectionInfo InitConnection(FWPM_CONNECTION* p, bool full = false);
	static WFPProviderContextInfo InitProviderContext(FWPM_PROVIDER_CONTEXT* p, bool full = false);

	template<typename TFilter = WFPFilterInfo> requires std::is_base_of_v<WFPFilterInfo, TFilter>
	static TFilter InitFilter(FWPM_FILTER* filter, bool full = false) {
		TFilter fi;
		fi.FilterKey = filter->filterKey;
		fi.FilterId = filter->filterId;
		fi.ProviderKey = filter->providerKey ? *filter->providerKey : GUID_NULL;
		fi.Name = ParseMUIString(filter->displayData.name);
		fi.Desc = ParseMUIString(filter->displayData.description);
		fi.ConditionCount = filter->numFilterConditions;
		fi.EffectiveWeight.Init(filter->effectiveWeight);
		fi.Flags = static_cast<WFPFilterFlags>(filter->flags);
		fi.LayerKey = filter->layerKey;
		fi.SubLayerKey = filter->subLayerKey;
		fi.Weight.Init(filter->weight);
		fi.Action.Type = static_cast<WFPActionType>(filter->action.type);
		fi.Action.FilterType = filter->action.filterType;
		fi.ProviderDataSize = filter->providerData.size;
		if (full) {
			fi.Conditions.reserve(fi.ConditionCount);
			for (uint32_t i = 0; i < fi.ConditionCount; i++) {
				auto const& cond = filter->filterCondition[i];
				WFPFilterCondition c;
				c.FieldKey = cond.fieldKey;
				c.MatchType = static_cast<WFPMatchType>(cond.matchType);
				c.Value.Init(cond.conditionValue);
				fi.Conditions.emplace_back(std::move(c));
			}
			if (fi.ProviderDataSize) {
				fi.ProviderData.resize(fi.ProviderDataSize);
				memcpy(fi.ProviderData.data(), filter->providerData.data, fi.ProviderDataSize);
			}
		}
		return fi;
	}

	template<typename TLayer = WFPLayerInfo> requires std::is_base_of_v<WFPLayerInfo, TLayer>
	static TLayer InitLayer(FWPM_LAYER* layer, bool includeFields) {
		TLayer li;
		li.Name = ParseMUIString(layer->displayData.name);
		li.Desc = ParseMUIString(layer->displayData.description);
		li.LayerKey = layer->layerKey;
		li.Flags = static_cast<WFPLayerFlags>(layer->flags);
		li.DefaultSubLayerKey = layer->defaultSubLayerKey;
		li.LayerId = layer->layerId;
		li.NumFields = layer->numFields;
		if (includeFields) {
			li.Fields.reserve(layer->numFields);
			for (UINT32 f = 0; f < layer->numFields; f++) {
				WFPFieldInfo fi;
				auto const& field = layer->field[f];
				fi.DataType = (WFPDataType)field.dataType;
				fi.Type = (WFPFieldType)field.type;
				fi.FieldKey = *field.fieldKey;
				li.Fields.emplace_back(fi);
			}
		}
		return li;
	}

	template<typename TLayer = WFPSubLayerInfo> requires std::is_base_of_v<WFPSubLayerInfo, TLayer>
	static TLayer InitSubLayer(FWPM_SUBLAYER* layer, bool full = false) {
		TLayer li;
		li.Name = ParseMUIString(layer->displayData.name);
		li.Desc = ParseMUIString(layer->displayData.description);
		li.SubLayerKey = layer->subLayerKey;
		li.Flags = static_cast<WFPSubLayerFlags>(layer->flags);
		li.Weight = layer->weight;
		li.ProviderKey = layer->providerKey ? *layer->providerKey : GUID_NULL;
		li.ProviderDataSize = layer->providerData.size;
		if (full && li.ProviderDataSize) {
			li.ProviderData.resize(li.ProviderDataSize);
			memcpy(li.ProviderData.data(), layer->providerData.data, layer->providerData.size);
		}
		return li;
	}

	static std::wstring PoorParseMUIString(std::wstring const& path);

	HANDLE m_hEngine{ nullptr };
	mutable DWORD m_LastError{ 0 };
};

