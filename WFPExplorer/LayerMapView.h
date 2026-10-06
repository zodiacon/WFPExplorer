#pragma once

#include <set>
#include <map>
#include <FrameView.h>
#include <CustomSplitterWindow.h>
#include <NodeGraphControl.h>
#include <WTLHelper.h>
#include <WFPEngine.h>
#include <WFPEnumerator.h>
#include "Interfaces.h"
#include "resource.h"

//
// a graph of WFP objects, of one of two kinds:
// layer map: a layer's sublayers in evaluation order (by weight), the filters of each sublayer in evaluation order
//   (by effective weight), and the callouts the filters hand traffic to
// callout map: a callout with its provider, the layer it applies to, and the filters that use it, by sublayer
//
class CLayerMapView : public CFrameView<CLayerMapView, IMainFrame> {
public:
	enum class MapKind { Layer, Callout };

	CLayerMapView(IMainFrame* frame, WFPEngine& engine, MapKind kind);

	void Refresh();
	void Select(GUID const& key);			// the layer or the callout to show
	void FocusCallout(GUID const& key);		// layer map: select a callout's node and bring it into view
	CString GetTitle() const;				// for the tab: the name of the layer or callout shown

	BEGIN_MSG_MAP(CLayerMapView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_ACTIVATE, OnActivate)
		MESSAGE_HANDLER(WM_REBUILD_GRAPH, OnRebuildGraph)
		MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
		MESSAGE_HANDLER(WTLHelper::ThemeChangedMessage, OnThemeChanged)
		NOTIFY_HANDLER(IdList, LVN_ITEMCHANGED, OnSelectionChanged)
		NOTIFY_HANDLER(IdGraph, NGCN_NODEDBLCLICK, OnNodeDoubleClick)
		CHAIN_MSG_MAP(BaseFrame)
	ALT_MSG_MAP(1)
		COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnRefresh)
		COMMAND_ID_HANDLER(ID_FILE_SAVE, OnSave)
		COMMAND_ID_HANDLER(ID_MAP_HIDEFIREWALL, OnToggleOption)
		COMMAND_ID_HANDLER(ID_MAP_COLORBYPROVIDER, OnToggleOption)
	END_MSG_MAP()

private:
	enum { IdList = 100, IdGraph };

	// posted to rebuild the graph outside of the graph control's notifications
	static constexpr UINT WM_REBUILD_GRAPH = WM_APP + 20;

	enum class NodeType {
		Layer, Sublayer, Filter, Callout, Provider, Summary, Note,
	};

	struct NodeInfo {
		NodeType Type;
		void* Data;			// FWPM_LAYER*, FWPM_SUBLAYER*, FWPM_FILTER*, FWPM_CALLOUT*, FWPM_PROVIDER* or nullptr
		GUID Key;			// the object's key; for a summary node, the sublayer it expands or collapses
	};

	struct GuidLess {
		bool operator()(GUID const& a, GUID const& b) const {
			return ::memcmp(&a, &b, sizeof(GUID)) < 0;
		}
	};

	// the columns of sublayers and their filters; the filters that use a callout are remembered for the edges to it
	struct CalloutUse {
		NodeGraphCtrl::NodeId Source;		// filter node, or the summary node of hidden filters
		size_t Column;
		float Y;
	};
	struct Columns {
		size_t Count{ 0 };
		size_t Filters{ 0 };
		float Bottom{ 0 };
		std::vector<GUID> CalloutOrder;		// in order of first use
		std::map<GUID, std::vector<CalloutUse>, GuidLess> CalloutUses;
		std::map<GUID, size_t, GuidLess> CalloutFilters;	// number of filters that use the callout
	};

	void FillList();
	GUID ItemKey(int index) const;
	void BuildGraph(bool fit);
	void BuildLayerGraph(NodeGraphCtrl::NodeGraphModel* model);
	void BuildCalloutGraph(NodeGraphCtrl::NodeGraphModel* model);
	Columns AddColumns(NodeGraphCtrl::NodeGraphModel* model, NodeGraphCtrl::NodeId parent, std::vector<FWPM_FILTER*> filters, float sublayerY);
	void AddLegend(NodeGraphCtrl::NodeGraphModel* model, float x);
	NodeGraphCtrl::NodeId AddNode(NodeGraphCtrl::NodeGraphModel* model, CString const& label, float x, float y,
		NodeGraphCtrl::NodeStyle const& style, std::wstring tooltip, NodeInfo const& info);
	NodeGraphCtrl::Edge* AddEdge(NodeGraphCtrl::NodeGraphModel* model, NodeGraphCtrl::NodeId from, NodeGraphCtrl::NodeId to,
		COLORREF color = RGB(140, 140, 140));
	void ApplyView();
	bool IsShown(FWPM_FILTER const* filter) const;
	NodeGraphCtrl::NodeStyle ProviderStyle(GUID const* provider);
	NodeGraphCtrl::NodeStyle StyleOf(FWPM_FILTER const* filter);
	CString GetProviderName(GUID const* key) const;
	FWPM_LAYER* FindLayer(GUID const& key) const;
	FWPM_CALLOUT* FindCallout(GUID const& key) const;
	FWPM_PROVIDER* FindProvider(GUID const& key) const;
	void ShowOptionsMenu(POINT const& pt);
	void ToggleOption(UINT id);		// ID_MAP_HIDEFIREWALL or ID_MAP_COLORBYPROVIDER
	void UpdateUI();
	void UpdateBackground();

	LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnActivate(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnToggleOption(WORD, WORD, HWND, BOOL&);
	LRESULT OnRebuildGraph(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnContextMenu(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnThemeChanged(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnSelectionChanged(int, LPNMHDR, BOOL&);
	LRESULT OnNodeDoubleClick(int, LPNMHDR, BOOL&);
	LRESULT OnRefresh(WORD, WORD, HWND, BOOL&);
	LRESULT OnSave(WORD, WORD, HWND, BOOL&);

	WFPEngine& m_Engine;
	MapKind m_Kind;
	CCustomSplitterWindow m_Splitter;
	CListViewCtrl m_List;
	NodeGraphCtrl::CNodeGraphControl m_Graph;

	// snapshot of the engine's objects; the graph and the list refer to them
	WFPObjectVector<FWPM_LAYER> m_Layers;
	WFPObjectVector<FWPM_SUBLAYER> m_Sublayers;
	WFPObjectVector<FWPM_FILTER> m_Filters;
	WFPObjectVector<FWPM_CALLOUT> m_Callouts;
	WFPObjectVector<FWPM_PROVIDER> m_Providers;

	GUID m_Key{ GUID_NULL };				// the layer or callout shown
	GUID m_FocusCallout{ GUID_NULL };		// to select and show once the graph has a size
	bool m_NeedFit{ false };				// fit the graph in view once it has a size
	bool m_HideFirewall, m_ColorByProvider;
	std::set<GUID, GuidLess> m_Expanded;	// sublayers that show all of their filters
	std::map<GUID, CString, GuidLess> m_ProviderNames;
	std::map<GUID, CString, GuidLess> m_CalloutNames;	// with the layer, for a name that more than one callout has
	std::map<GUID, int, GuidLess> m_ProviderColors;		// index into the palette
	std::set<GUID, GuidLess> m_UsedProviders;			// the providers of the nodes in the graph, for the legend
	std::unordered_map<NodeGraphCtrl::NodeId, NodeInfo> m_NodeInfo;
};
