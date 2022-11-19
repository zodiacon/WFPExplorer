#pragma once

class WFPEngine;
struct WFPLayerInfo;
struct WFPFilterInfo;

struct WFPHelper abstract final {
	static CString GetProviderName(WFPEngine const& engine, GUID const& key);
	static CString GetFilterName(WFPEngine const& engine, GUID const& key);
	static CString GetLayerName(WFPEngine const& engine, GUID const& key);
	static CString GetSublayerName(WFPEngine const& engine, GUID const& key);
	static int ShowLayerProperties(WFPEngine& engine, WFPLayerInfo& layer);
	static int ShowFilterProperties(WFPEngine& engine, WFPFilterInfo& layer);
};

