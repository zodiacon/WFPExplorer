#pragma once

class WFPEngine;
namespace NodeGraphCtrl {
	class CNodeGraphControl;
}

struct WFPHelper abstract final {
	static CString GetProviderName(WFPEngine const& engine, GUID const& key);
	static CString GetFilterName(WFPEngine const& engine, GUID const& key);
	static CString GetLayerName(WFPEngine const& engine, GUID const& key);
	static CString GetCalloutName(WFPEngine const& engine, GUID const& key);
	static CString GetSublayerName(WFPEngine const& engine, GUID const& key);
	static int ShowLayerProperties(WFPEngine& engine, FWPM_LAYER* layer);
	// the menu of a layer (or a callout) in the views other than the Layers (Callouts) view; returns the chosen command, or 0
	static UINT TrackMapMenu(HWND hWnd, POINT const& pt, bool callout = false);
	// asks where, and saves a map as an image, an SVG file, or a map file (that File > Open shows again)
	static void SaveMap(HWND hWnd, NodeGraphCtrl::CNodeGraphControl& graph, CString name);
	// fits a graph in view: a small graph isn't blown up, a large one shows its top-left part at a readable size
	static void FitGraph(NodeGraphCtrl::CNodeGraphControl& graph);
	static int ShowFilterProperties(WFPEngine& engine, FWPM_FILTER* filter);
	static int ShowSublayerProperties(WFPEngine& engine, FWPM_SUBLAYER* sublayer);
	static int ShowProviderProperties(WFPEngine& engine, FWPM_PROVIDER* provider);
	static int ShowCalloutProperties(WFPEngine& engine, FWPM_CALLOUT* callout);
	static bool Sort(FWP_VALUE const& v1, FWP_VALUE const& v2, bool asc);
};

