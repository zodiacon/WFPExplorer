#pragma once

class WFPEngine;

struct WFPHelper abstract final {
	static CString GetProviderName(WFPEngine const& engine, GUID const& key);
	static CString GetFilterName(WFPEngine const& engine, GUID const& key);
	static CString GetLayerName(WFPEngine const& engine, GUID const& key);
	static CString GetCalloutName(WFPEngine const& engine, GUID const& key);
	static CString GetSublayerName(WFPEngine const& engine, GUID const& key);
	static int ShowLayerProperties(WFPEngine& engine, FWPM_LAYER* layer);
	static int ShowFilterProperties(WFPEngine& engine, FWPM_FILTER* filter);
	static int ShowSublayerProperties(WFPEngine& engine, FWPM_SUBLAYER* sublayer);
	static int ShowProviderProperties(WFPEngine& engine, FWPM_PROVIDER* provider);
	static int ShowCalloutProperties(WFPEngine& engine, FWPM_CALLOUT* callout);
};

