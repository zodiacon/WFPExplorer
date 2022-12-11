#pragma once

class WFPEngine;
struct WFPLayerInfo;
struct WFPFilterInfo;
struct WFPProviderInfo;
struct WFPSubLayerInfo;

struct WFPHelper abstract final {
	static CString GetProviderName(WFPEngine const& engine, GUID const& key);
	static CString GetFilterName(WFPEngine const& engine, GUID const& key);
	static CString GetLayerName(WFPEngine const& engine, GUID const& key);
	static CString GetSublayerName(WFPEngine const& engine, GUID const& key);
	static int ShowLayerProperties(WFPEngine& engine, WFPLayerInfo const& layer);
	static int ShowFilterProperties(WFPEngine& engine, WFPFilterInfo const& filter);
	static int ShowSublayerProperties(WFPEngine& engine, WFPSubLayerInfo& sublayer);
	static int ShowProviderProperties(WFPEngine& engine, FWPM_PROVIDER* provider);
};

