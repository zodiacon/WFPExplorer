#include "pch.h"
#include "LayersView.h"
#include "StringHelper.h"
#include <SortHelper.h>
#include "WFPHelper.h"
#include "LayerGeneralPage.h"
#include <WFPEnumerators.h>
#include <ClipboardHelper.h>
#include <ThemeHelper.h>

CLayersView::CLayersView(IMainFrame* frame, WFPEngine& engine) : CGenericListViewBase(frame), m_Engine(engine) {
}

LRESULT CLayersView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	m_hWndClient = m_List.Create(m_hWnd, rcDefault, nullptr,
		WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | LVS_OWNERDATA | LVS_REPORT | LVS_SHOWSELALWAYS);
	m_List.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"Layer Key", 0, 300, ColumnType::Key, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Layer ID", LVCFMT_RIGHT, 70, ColumnType::Id, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Name", 0, 250, ColumnType::Name);
	cm->AddColumn(L"Flags", LVCFMT_LEFT, 250, ColumnType::Flags);
	cm->AddColumn(L"Fields", LVCFMT_RIGHT, 60, ColumnType::Fields, ColumnFlags::Visible | ColumnFlags::Numeric);
	//cm->AddColumn(L"Filters", LVCFMT_RIGHT, 60, ColumnType::Filters);
	//cm->AddColumn(L"Callouts", LVCFMT_RIGHT, 60, ColumnType::Callouts);
	cm->AddColumn(L"Default Sublayer", 0, 250, ColumnType::DefaultSubLayer);
	cm->AddColumn(L"Description", 0, 180, ColumnType::Desc);

	CImageList images;
	images.Create(16, 16, ILC_COLOR32 | ILC_MASK, 1, 1);
	images.AddIcon(AtlLoadIconImage(IDI_LAYERS, 0, 16, 16));
	m_List.SetImageList(images, LVSIL_SMALL);

	Refresh();

	return 0;
}

LRESULT CLayersView::OnActivate(UINT, WPARAM active, LPARAM, BOOL&) {
	if (active) {
		UpdateUI();
		return 1;
	}
	return 0;
}

LRESULT CLayersView::OnRefresh(WORD, WORD, HWND, BOOL&) {
	Refresh();
	return 0;
}

LRESULT CLayersView::OnProperties(WORD, WORD, HWND, BOOL&) {
	auto& layer = m_Layers[m_List.GetSelectedIndex()];
	WFPHelper::ShowLayerProperties(m_Engine, layer.Data);
	return 0;
}

void CLayersView::Refresh() {
	WFPLayerEnumerator enumerator(m_Engine.Handle());
	m_Layers = enumerator.Next<LayerInfo>();
	Sort(m_List);
	m_List.SetItemCountEx((int)m_Layers.size(), LVSICF_NOSCROLL);
	Frame()->SetStatusText(4, std::format(L"{} Layers", m_Layers.size()).c_str());
}

CString CLayersView::GetColumnText(HWND, int row, int col) {
	auto& layer = m_Layers[row];
	auto info = layer.Data;
	switch (GetColumnManager(m_List)->GetColumnTag<ColumnType>(col)) {
		case ColumnType::Key: return StringHelper::GuidToString(info->layerKey);
		case ColumnType::DefaultSubLayer:
			if (layer.DefaultSublayer.IsEmpty()) {
				if (info->defaultSubLayerKey != GUID_NULL) {
					auto sl = m_Engine.GetSublayerByKey(info->defaultSubLayerKey);
					if (sl)
						layer.DefaultSublayer = StringHelper::ParseMUIString(sl->displayData.name);
					else
						layer.DefaultSublayer = StringHelper::GuidToString(info->defaultSubLayerKey);
				}
			}
			return layer.DefaultSublayer;

		case ColumnType::Filters: return std::to_wstring(layer.FilterCount).c_str();
		case ColumnType::Callouts: return std::to_wstring(layer.CalloutCount).c_str();

		case ColumnType::Name: return layer.Name();
		case ColumnType::Desc: return layer.Desc();
		case ColumnType::Flags: 
			if (info->flags == 0)
				return L"0";
			return std::format(L"0x{:X} ({})", info->flags, 
				(PCWSTR)StringHelper::WFPLayerFlagsToString(info->flags)).c_str();

		case ColumnType::Fields: return std::to_wstring(info->numFields).c_str();
		case ColumnType::Id: return std::to_wstring(info->layerId).c_str();
	}
	return CString();
}

void CLayersView::DoSort(SortInfo const* si) {
	auto col = GetColumnManager(m_List)->GetColumnTag<ColumnType>(si->SortColumn);
	auto asc = si->SortAscending;

	auto compare = [&](auto& layer1, auto& layer2) {
		auto l1 = layer1.Data, l2 = layer2.Data;
		switch (col) {
			case ColumnType::Key: return SortHelper::Sort(StringHelper::GuidToString(l1->layerKey), StringHelper::GuidToString(l2->layerKey), asc);
			case ColumnType::Name: return SortHelper::Sort(layer1.Name(), layer2.Name(), asc);
			case ColumnType::Desc: return SortHelper::Sort(layer1.Desc(), layer2.Desc(), asc);
			case ColumnType::Flags: return SortHelper::Sort(l1->flags, l2->flags, asc);
			case ColumnType::Fields: return SortHelper::Sort(l1->numFields, l2->numFields, asc);
			case ColumnType::DefaultSubLayer: return SortHelper::Sort(layer1.DefaultSublayer, layer2.DefaultSublayer, asc);
			case ColumnType::Id: return SortHelper::Sort(l1->layerId, l2->layerId, asc);
		}
		return false;
	};
	std::ranges::sort(m_Layers, compare);
}

int CLayersView::GetSaveColumnRange(HWND, int&) const {
	return 1;
}

int CLayersView::GetRowImage(HWND, int row, int col) const {
	return 0;
}

void CLayersView::UpdateUI() {
	auto& ui = Frame()->UI();
	int selected = m_List.GetSelectedCount();
	ui.UIEnable(ID_EDIT_PROPERTIES, selected == 1);
	ui.UIEnable(ID_EDIT_COPY, selected > 0);
}

int CLayersView::GetFilterCount(LayerInfo& layer) const {
	if (layer.FilterCount < 0)
		layer.FilterCount = m_Engine.GetFilterCount(layer.Data->layerKey);
	return layer.FilterCount;
}

int CLayersView::GetCalloutCount(LayerInfo& layer) const {
	if (layer.CalloutCount < 0)
		layer.CalloutCount = m_Engine.GetCalloutCount(layer.Data->layerKey);
	return layer.CalloutCount;
}

void CLayersView::OnStateChanged(HWND, int from, int to, UINT oldState, UINT newState) {
	if ((newState & LVIS_SELECTED) || (oldState & LVIS_SELECTED))
		UpdateUI();
}

bool CLayersView::OnDoubleClickList(HWND, int, int, POINT const&) {
	LRESULT result = 0;
	return ProcessWindowMessage(m_hWnd, WM_COMMAND, ID_EDIT_PROPERTIES, 0, result, 1);
}

CString CLayersView::GetDefaultSaveFile() const {
	return L"layers.csv";
}

CString const& CLayersView::LayerInfo::Name() const {
	if (m_Name.IsEmpty())
		m_Name = StringHelper::ParseMUIString(Data->displayData.name);
	return m_Name;
}

CString const& CLayersView::LayerInfo::Desc() const {  
	if (m_Desc.IsEmpty())
		m_Desc = StringHelper::ParseMUIString(Data->displayData.description);
	return m_Desc;
}

LRESULT CLayersView::OnCopy(WORD, WORD, HWND, BOOL&) {
	auto text = ListViewHelper::GetSelectedRowsAsString(m_List, L",");
	ClipboardHelper::CopyText(m_hWnd, text);

	return 0;
}

bool CLayersView::OnRightClickList(HWND, int row, int col, POINT const& pt) {
	if (row < 0)
		return false;

	CMenu menu;
	menu.LoadMenu(IDR_CONTEXT);

	return Frame()->TrackPopupMenu(menu.GetSubMenu(1), 0, pt.x, pt.y);
}
