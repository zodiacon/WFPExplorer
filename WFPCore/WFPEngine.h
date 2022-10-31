#pragma once

#include <vector>
#include <atlsecurity.h>
#include <memory>
#include <type_traits>
#include <concepts>

struct WFPSessionInfo {
    GUID SessionKey;
    std::wstring Name;
    std::wstring Desc;
    UINT32 flags;
    UINT32 TxWaitTimeoutInMSec;
    DWORD ProcessId;
    BYTE Sid[SECURITY_MAX_SID_SIZE];
    std::wstring UserName;
    DWORD Flags;
    bool KernelMode;
};

struct WFPProviderInfo {
    GUID ProviderKey;
    std::wstring Name;
    std::wstring Desc;
    std::wstring ServiceName;
    UINT32 Flags;
    ULONG ProviderDataSize;
    std::unique_ptr<BYTE[]> ProviderData;
};

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
    SINGLE_DATA_TYPE_MAX = 0xff,
    V4_ADDR_MASK,
    V6_ADDR_MASK,
    RANGE_TYPE,
};

struct WFPValue {
    WFPDataType Type;
    union {
        UINT8 uint8;
        UINT16 uint16;
        UINT32 uint32;
        UINT64* uint64;
        INT8 int8;
        INT16 int16;
        INT32 int32;
        INT64* int64;
        float float32;
        double* double64;
        FWP_BYTE_ARRAY16* byteArray16;
        FWP_BYTE_BLOB* byteBlob;
        SID* sid;
        FWP_BYTE_BLOB* sd;
        FWP_TOKEN_INFORMATION* tokenInformation;
        FWP_BYTE_BLOB* tokenAccessInformation;
        LPWSTR unicodeString;
        FWP_BYTE_ARRAY6* byteArray6;
    };
};

static_assert(sizeof(WFPValue) == sizeof(FWP_VALUE));

struct WFPFieldInfo {
    GUID FieldKey;
    WFPFieldType Type;
    WFPDataType DataType;
};

struct WFPLayerInfo {
    GUID LayerKey;
    std::wstring Name;
    std::wstring Desc;
    UINT32 Flags;
    std::vector<WFPFieldInfo> Fields;
    GUID DefaultSubLayerKey;
    UINT16 LayerId;
};

struct WFPSubLayerInfo {
    GUID SubLayerKey;
    std::wstring Name;
    std::wstring Desc;
    UINT32 Flags;
    GUID ProviderKey;
    std::vector<BYTE> ProviderData;
    UINT16 Weight;
};

enum class WFPMatchType {
    EQUAL,
    GREATER,
    LESS,
    GREATER_OR_EQUAL,
    LESS_OR_EQUAL,
    RANGE,
    FLAGS_ALL_SET,
    FLAGS_ANY_SET,
    FLAGS_NONE_SET,
    EQUAL_CASE_INSENSITIVE,
    NOT_EQUAL,
    PREFIX,
    NOT_PREFIX,
};

struct WFPRange {
    WFPValue Low;
    WFPValue High;
};

struct WFPConditionValue {
    WFPDataType Type;
    union {
        UINT8 uint8;
        UINT16 uint16;
        UINT32 uint32;
        UINT64* uint64;
        INT8 int8;
        INT16 int16;
        INT32 int32;
        INT64* int64;
        float float32;
        double* double64;
        FWP_BYTE_ARRAY16* byteArray16;
        FWP_BYTE_BLOB* byteBlob;
        SID* sid;
        FWP_BYTE_BLOB* sd;
        FWP_TOKEN_INFORMATION* tokenInformation;
        FWP_BYTE_BLOB* tokenAccessInformation;
        LPWSTR unicodeString;
        FWP_BYTE_ARRAY6* byteArray6;
        FWP_V4_ADDR_AND_MASK* v4AddrMask;
        FWP_V6_ADDR_AND_MASK* v6AddrMask;
        WFPRange* rangeValue;
    };
};

struct WFPFilterCondition {
    GUID FieldKey;
    WFPMatchType MatchType;
    WFPConditionValue Value;
};

struct WFPFilterInfo {
    GUID FilterKey;
    std::wstring Name;
    std::wstring Desc;
    UINT32 Flags;
    GUID ProviderKey;
    std::vector<BYTE> ProviderData;
    GUID LayerKey;
    GUID SubLayerKey;
    WFPValue Weight;
    UINT32 ConditionCount;
    std::vector<WFPFilterCondition> Conditions;
    FWPM_ACTION0 action;
    union {
        UINT64 RawContext;
        GUID ProviderContextKey;
    };
    UINT64 FilterId;
    WFPValue EffectiveWeight;
};

struct WFPCalloutInfo {
    GUID CalloutKey;
    std::wstring Name;
    std::wstring Desc;
    UINT32 Flags;
    GUID ProviderKey;
    std::vector<BYTE> ProviderData;
    GUID ApplicableLayer;
    UINT32 CalloutId;
};

class WFPEngine {
public:
	bool Open(DWORD auth = RPC_C_AUTHN_WINNT);
	~WFPEngine();
	void Close();
	DWORD LastError() const;

    //
    // enumerations
    //

    template<typename TSession = WFPSessionInfo> requires std::is_base_of_v<WFPSessionInfo, TSession>
    std::vector<TSession> EnumSessions() const {
        HANDLE hEnum;
        std::vector<TSession> info;
        m_LastError = FwpmSessionCreateEnumHandle(m_hEngine, nullptr, &hEnum);
        if (m_LastError != ERROR_SUCCESS)
            return info;

        UINT32 count;
        FWPM_SESSION** sessions;
        if ((m_LastError = FwpmSessionEnum(m_hEngine, hEnum, 128, &sessions, &count)) == ERROR_SUCCESS) {
            info.reserve(count);
            for (UINT32 i = 0; i < count; i++) {
                auto session = sessions[i];
                TSession si;
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

    template<typename TFilter = WFPFilterInfo> requires std::is_base_of_v<WFPFilterInfo, TFilter>
    std::vector<TFilter> EnumFilters(bool includeConditions = false) const {
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
                TFilter fi;
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
    std::vector<WFPLayerInfo> EnumLayers() const;
    std::vector<WFPSubLayerInfo> EnumSubLayers() const;
    std::vector<WFPProviderInfo> EnumProviders(bool includeData = false) const;
    std::vector<WFPCalloutInfo> EnumCallouts() const;

    //
    // providers API
    //
    WFPProviderInfo GetProviderByKey(GUID const& guid) const;

    //
    // helpers
    //
    static std::wstring ParseMUIString(PCWSTR input);
    static WFPProviderInfo InitProvider(FWPM_PROVIDER* p, bool includeData = false);

private:
	HANDLE m_hEngine{ nullptr };
	mutable DWORD m_LastError{ 0 };
};

