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

    operator bool() const;
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
        UINT64 uint64;
        INT8 int8;
        INT16 int16;
        INT32 int32;
        INT64 int64;
        float float32;
        double double64;
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

WFPValue WFPValueInit(FWP_VALUE const& value);

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
    UINT32 NumFields;
    std::vector<WFPFieldInfo> Fields;
    GUID DefaultSubLayerKey;
    UINT16 LayerId;

    operator bool() const;
};

struct WFPSubLayerInfo {
    GUID SubLayerKey;
    std::wstring Name;
    std::wstring Desc;
    UINT32 Flags;
    GUID ProviderKey;
    std::vector<BYTE> ProviderData;
    UINT16 Weight;

    operator bool() const;
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
                auto fi = InitFilter<TFilter>(filter, includeConditions);
                info.emplace_back(std::move(fi));
            }
            FwpmFreeMemory((void**)&filters);
            m_LastError = FwpmFilterDestroyEnumHandle(m_hEngine, hEnum);
        }
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
        }
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
        }
        return info;

    }

    std::vector<WFPProviderInfo> EnumProviders(bool includeData = false) const;

    template<typename TCallout = WFPCalloutInfo> requires std::is_base_of_v<WFPCalloutInfo, TCallout>
    std::vector<TCallout> EnumCallouts() const {
        HANDLE hEnum;
        std::vector<TCallout> info;
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
                auto ci = InitCallout(c);
                info.emplace_back(std::move(ci));
            }
            FwpmFreeMemory((void**)&callouts);
            m_LastError = FwpmCalloutDestroyEnumHandle(m_hEngine, hEnum);
        }

        return info;
    }

    //
    // providers API
    //
    WFPProviderInfo GetProviderByKey(GUID const& key) const;

    //
    // Filters API
    //
    WFPFilterInfo GetFilterByKey(GUID const& key) const;
    WFPFilterInfo GetFilterById(UINT64 id) const;

    //
    // layer API
    //
    WFPLayerInfo GetLayerByKey(GUID const& key) const;

    //
    // sublayer API
    //
    WFPSubLayerInfo GetSubLayerByKey(GUID const& key) const;
    WFPSubLayerInfo GetSubLayerById(UINT16 id) const;


    //
    // helpers
    //
    static std::wstring ParseMUIString(PCWSTR input);
    static WFPProviderInfo InitProvider(FWPM_PROVIDER* p, bool includeData = false);
    
    template<typename TFilter = WFPFilterInfo> requires std::is_base_of_v<WFPFilterInfo, TFilter>
    static TFilter InitFilter(FWPM_FILTER* filter, bool includeConditions = false) {
        TFilter fi;
        fi.FilterKey = filter->filterKey;
        fi.FilterId = filter->filterId;
        fi.ProviderKey = filter->providerKey ? *filter->providerKey : GUID_NULL;
        fi.Name = ParseMUIString(filter->displayData.name);
        fi.Desc = ParseMUIString(filter->displayData.description);
        fi.ConditionCount = filter->numFilterConditions;
        fi.EffectiveWeight = WFPValueInit(filter->effectiveWeight);
        fi.Flags = filter->flags;
        fi.LayerKey = filter->layerKey;
        fi.SubLayerKey = filter->subLayerKey;
        fi.Weight = WFPValueInit(filter->weight);
        if (includeConditions) {
            // TODO
        }
        return fi;
    }

    template<typename TLayer = WFPLayerInfo> requires std::is_base_of_v<WFPLayerInfo, TLayer>
    static TLayer InitLayer(FWPM_LAYER* layer, bool includeFields) {
        TLayer li;
        li.Name = ParseMUIString(layer->displayData.name);
        li.Desc = ParseMUIString(layer->displayData.description);
        li.LayerKey = layer->layerKey;
        li.Flags = layer->flags;
        li.DefaultSubLayerKey = layer->defaultSubLayerKey;
        li.LayerId = layer->layerId;
        li.NumFields = layer->numFields;
        if (includeFields) {
            li.Fields.reserve(layer->numFields);
            for (UINT32 f = 0; f < layer->numFields; f++) {
                WFPFieldInfo fi;
                auto& field = layer->field[f];
                fi.DataType = (WFPDataType)field.dataType;
                fi.Type = (WFPFieldType)field.type;
                fi.FieldKey = *field.fieldKey;
                li.Fields.emplace_back(fi);
            }
        }
        return li;
    }

    template<typename TLayer = WFPSubLayerInfo> requires std::is_base_of_v<WFPSubLayerInfo, TLayer>
    static TLayer InitSubLayer(FWPM_SUBLAYER* layer) {
        TLayer li;
        li.Name = ParseMUIString(layer->displayData.name);
        li.Desc = ParseMUIString(layer->displayData.description);
        li.SubLayerKey = layer->subLayerKey;
        li.Flags = layer->flags;
        li.Weight = layer->weight;
        li.ProviderKey = layer->providerKey ? *layer->providerKey : GUID_NULL;
        li.ProviderData.resize(layer->providerData.size);
        memcpy(li.ProviderData.data(), layer->providerData.data, layer->providerData.size);
        return li;
    }

    template<typename TCallout = WFPCalloutInfo> requires std::is_base_of_v<WFPCalloutInfo, TCallout>
    static TCallout InitCallout(FWPM_CALLOUT* c) {
        TCallout ci;
        ci.Name = ParseMUIString(c->displayData.name);
        ci.Desc = ParseMUIString(c->displayData.description);
        ci.ProviderKey = c->providerKey ? *c->providerKey : GUID_NULL;
        ci.Flags = c->flags;
        ci.CalloutKey = c->calloutKey;
        ci.ProviderData.resize(c->providerData.size);
        memcpy(ci.ProviderData.data(), c->providerData.data, c->providerData.size);
        ci.CalloutId = c->calloutId;
        ci.ApplicableLayer = c->applicableLayer;
        return ci;
    }

private:
    static std::wstring PoorParseMUIString(std::wstring const& path);

	HANDLE m_hEngine{ nullptr };
	mutable DWORD m_LastError{ 0 };
};

