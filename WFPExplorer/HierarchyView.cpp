#include "pch.h"
#include "HierarchyView.h"
#include "WFPHelper.h"
#include "StringHelper.h"
#include <ranges>
#include "AppSettings.h"
#include <WFPEnumerators.h>

CHierarchyView::CHierarchyView(IMainFrame* frame, WFPEngine& engine) : CFrameView(frame), m_Engine(engine) {
}

void CHierarchyView::Refresh() {
	BuildTree();
}

void CHierarchyView::OnTreeSelChanged(HWND tree, HTREEITEM /* hOld */, HTREEITEM hNew) {
	auto type = GetItemData<TreeItemType>(tree, hNew);
	HWND hNewView = nullptr;
	auto hOldView = m_Splitter.GetSplitterPane(SPLIT_PANE_RIGHT);
	switch (type) {
		case TreeItemType::Layers:
			hNewView = m_LayersView->m_hWnd;
			break;

		case TreeItemType::Filters:
		{
			hNewView = m_FiltersView->m_hWnd;
			auto hLayer = m_Tree.GetParentItem(hNew);
			m_FiltersView->SetLayer(m_LayersMap[hLayer]);
			m_FiltersView->Refresh();
			break;
		}

		case TreeItemType::Callouts:
		{
			hNewView = m_CalloutsView->m_hWnd;
			auto hLayer = m_Tree.GetParentItem(hNew);
			m_CalloutsView->SetLayer(m_LayersMap[hLayer]);
			m_CalloutsView->Refresh();
			break;
		}

		default:
			m_Splitter.SetSinglePaneMode(0);
			break;
	}
	if (hNewView) {
		m_Splitter.SetSinglePaneMode();
		::ShowWindow(hNewView, SW_SHOW);
		m_Splitter.SetSplitterPane(SPLIT_PANE_RIGHT, hNewView);
	}
	if (hOldView && hOldView != hNewView)
		::ShowWindow(hOldView, SW_HIDE);
	UpdateUI();
}

bool CHierarchyView::OnTreeDoubleClick(HWND tree, HTREEITEM hItem) {
	return ShowProperties(hItem);
}

void CHierarchyView::BuildTree() {
	CWaitCursor wait;
	m_Tree.SetRedraw(FALSE);
	m_Tree.DeleteAllItems();

	using namespace std;

	auto hLayers = InsertTreeItem(m_Tree, L"Layers", 1, TreeItemType::Layers, TVI_ROOT);
	auto filters = WFPFilterEnumerator(m_Engine.Handle()).Next(8192);
	WFPCalloutEnumerator cenum(m_Engine.Handle());
	auto callouts = cenum.Next(1024);
	m_LayersMap.clear();
	m_FiltersMap.clear();
	m_CalloutsMap.clear();
	auto hideEmptyLayers = AppSettings::Get().HideEmptyLayers();
	auto layers = WFPLayerEnumerator(m_Engine.Handle()).Next(1024);

	for (auto layer : layers) {
		auto hLayer = InsertTreeItem(m_Tree, WFPHelper::GetLayerName(m_Engine, layer->layerKey), 1, TreeItemType::Layer, hLayers, TVI_SORT);
		m_LayersMap.insert({ hLayer, layer->layerKey });
		{
			auto view = (filters | views::filter([&](auto& f) { return f->layerKey == layer->layerKey; }));
			if (!view.empty()) {
				auto hFilters = InsertTreeItem(m_Tree, L"Filters", 0, TreeItemType::Filters, hLayer, TVI_LAST);
				uint32_t count = 0;
				for (auto& v : view) {
					auto name = WFPHelper::GetFilterName(m_Engine, v->filterKey);
					if (name[0] != L'{')
						name += L" " + StringHelper::GuidToString(v->filterKey);
					auto hFilter = InsertTreeItem(m_Tree, name, 0, TreeItemType::Filter, hFilters, TVI_SORT);
					m_FiltersMap.insert({ hFilter, v->filterKey });
					count++;
				}
				m_Tree.SetItemText(hFilters, std::format(L"Filters ({})", count).c_str());
			}
		}
		{
			auto view = callouts | views::filter([&](auto& c) { return c->applicableLayer == layer->layerKey; });
			if (!view.empty()) {
				auto hCallouts = InsertTreeItem(m_Tree, L"Callouts", 2, TreeItemType::Callouts, hLayer, TVI_LAST);
				uint32_t count = 0;
				for (auto& v : view) {
					CString name = v->displayData.name;
					if (name[0] != L'{')
						name += L" " + StringHelper::GuidToString(v->calloutKey);
					auto hCallout = InsertTreeItem(m_Tree, name, 2, TreeItemType::Callout, hCallouts, TVI_SORT);
					m_CalloutsMap.insert({ hCallout, v->calloutKey });
					count++;
				}
				m_Tree.SetItemText(hCallouts, std::format(L"Callouts ({})", count).c_str());
			}
		}
		if (hideEmptyLayers && m_Tree.GetChildItem(hLayer) == nullptr) {
			//
			// layer has no callouts nor filters
			//
			m_Tree.DeleteItem(hLayer);
			m_LayersMap.erase(hLayer);
		}
	}

	m_Tree.Expand(hLayers, TVE_EXPAND);
	m_Tree.SelectItem(hLayers);
	m_Tree.SetRedraw();
}

void CHierarchyView::UpdateUI() {
	auto& ui = Frame()->UI();
	auto hItem = m_Tree.GetSelectedItem();
	auto type = hItem ? GetItemData<TreeItemType>(m_Tree, hItem) : TreeItemType::None;
	ui.UIEnable(ID_EDIT_PROPERTIES, type == TreeItemType::Filter || type == TreeItemType::Layer);
	ui.UIEnable(ID_EDIT_COPY, type != TreeItemType::None);
}

bool CHierarchyView::ShowProperties(HTREEITEM hItem) {
	auto type = GetItemData<TreeItemType>(m_Tree, hItem);
	switch (type) {
		case TreeItemType::Layer:
			WFPHelper::ShowLayerProperties(m_Engine, *m_Engine.GetLayerByKey(m_LayersMap[hItem]));
			return true;

		case TreeItemType::Filter:
			WFPHelper::ShowFilterProperties(m_Engine, *m_Engine.GetFilterByKey(m_FiltersMap[hItem]));
			return true;

		case TreeItemType::Callout:
			//WFPHelper::ShowCalloutProperties(m_Engine, *m_Engine.GetCalloutByKey(m_CalloutsMap[hItem], false));
			return true;
	}
	return false;
}

LRESULT CHierarchyView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	m_hWndClient = m_Splitter.Create(m_hWnd, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
	m_Tree.Create(m_Splitter, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | TVS_SHOWSELALWAYS | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT);

	m_LayersView = new CLayersView(Frame(), m_Engine);
	m_LayersView->Create(m_Splitter, rcDefault, nullptr, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);

	m_FiltersView = new CFiltersView(Frame(), m_Engine);
	m_FiltersView->Create(m_Splitter, rcDefault, nullptr, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);

	m_CalloutsView = new CCalloutsView(Frame(), m_Engine);
	m_CalloutsView->Create(m_Splitter, rcDefault, nullptr, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);

	CImageList images;
	images.Create(16, 16, ILC_COLOR32, 8, 4);
	UINT icons[] = {
		IDI_FILTER, IDI_LAYERS, IDI_CALLOUT,
	};
	for (auto icon : icons)
		images.AddIcon(AtlLoadIconImage(icon, 0, 16, 16));
	m_Tree.SetImageList(images);

	m_Splitter.SetSplitterPane(0, m_Tree);
	m_Splitter.SetSplitterPosPct(20);

	Refresh();

	return 0;
}

LRESULT CHierarchyView::OnRefresh(WORD, WORD, HWND, BOOL&) {
	Refresh();

	return 0;
}

LRESULT CHierarchyView::OnSetFocus(UINT, WPARAM, LPARAM, BOOL&) {
	m_Tree.SetFocus();

	return 0;
}

LRESULT CHierarchyView::OnProperties(WORD, WORD, HWND, BOOL& handled) {
	auto hWnd = ::GetFocus();
	if (hWnd == m_Tree)
		ShowProperties(m_Tree.GetSelectedItem());
	else
		handled = FALSE;
	return 0;
}
