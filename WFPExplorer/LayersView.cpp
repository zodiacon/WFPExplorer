#include "pch.h"
#include "LayersView.h"
#include "StringHelper.h"
#include <SortHelper.h>
#include "WFPHelper.h"
#include "LayerGeneralPage.h"

CLayersView::CLayersView(IMainFrame* frame, WFPEngine& engine) : CFrameView(frame), m_Engine(engine) {
}

LRESULT CLayersView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	m_hWndClient = m_List.Create(m_hWnd, rcDefault, nullptr,
		WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | LVS_OWNERDATA | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL);
	m_List.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"Layer Key", 0, 280, ColumnType::Key);
	cm->AddColumn(L"Layer ID", LVCFMT_RIGHT, 70, ColumnType::Id);
	cm->AddColumn(L"Name", 0, 250, ColumnType::Name);
	cm->AddColumn(L"Flags", LVCFMT_LEFT, 180, ColumnType::Flags);
	cm->AddColumn(L"Fields", LVCFMT_RIGHT, 60, ColumnType::Fields);
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
	WFPHelper::ShowLayerProperties(m_Engine, layer);
	return 0;
}

void CLayersView::Refresh() {
	m_Layers = m_Engine.EnumLayers<LayerInfo>();
	Sort(m_List);
	m_List.SetItemCountEx((int)m_Layers.size(), LVSICF_NOSCROLL);
	Frame()->SetStatusText(4, std::format(L"{} Layers", m_Layers.size()).c_str());
}

CString CLayersView::GetColumnText(HWND, int row, int col) {
	auto& info = m_Layers[row];
	switch (GetColumnManager(m_List)->GetColumnTag<ColumnType>(col)) {
		case ColumnType::Key: return StringHelper::GuidToString(info.LayerKey);
		case ColumnType::DefaultSubLayer:
			if (info.DefaultSublayer.IsEmpty()) {
				if (info.DefaultSubLayerKey != GUID_NULL) {
					auto sl = m_Engine.GetSublayerByKey(info.DefaultSubLayerKey);
					if (sl && !sl.value().Name.empty())
						info.DefaultSublayer = sl.value().Name.c_str();
					else
						info.DefaultSublayer = StringHelper::GuidToString(info.DefaultSubLayerKey);
				}
			}
			return info.DefaultSublayer;

		case ColumnType::Filters: return std::to_wstring(info.FilterCount).c_str();
		case ColumnType::Callouts: return std::to_wstring(info.CalloutCount).c_str();

		case ColumnType::Name: return info.Name.c_str();
		case ColumnType::Desc: return info.Desc.c_str();
		case ColumnType::Flags: 
			if (info.Flags == WFPLayerFlags::None)
				return L"0";
			return std::format(L"0x{:X} ({})", (UINT32)info.Flags, 
				(PCWSTR)StringHelper::WFPLayerFlagsToString(info.Flags)).c_str();

		case ColumnType::Fields: return std::to_wstring(info.NumFields).c_str();
		case ColumnType::Id: return std::to_wstring(info.LayerId).c_str();
	}
	return CString();
}

void CLayersView::DoSort(SortInfo const* si) {
	if (si == nullptr)
		return;
	auto col = GetColumnManager(m_List)->GetColumnTag<ColumnType>(si->SortColumn);
	auto asc = si->SortAscending;

	auto compare = [&](auto& l1, auto& l2) {
		switch (col) {
			case ColumnType::Key: return SortHelper::Sort(StringHelper::GuidToString(l1.LayerKey), StringHelper::GuidToString(l2.LayerKey), asc);
			case ColumnType::Name: return SortHelper::Sort(l1.Name, l2.Name, asc);
			case ColumnType::Desc: return SortHelper::Sort(l1.Desc, l2.Desc, asc);
			case ColumnType::Flags: return SortHelper::Sort(l1.Flags, l2.Flags, asc);
			case ColumnType::Fields: return SortHelper::Sort(l1.NumFields, l2.NumFields, asc);
			case ColumnType::DefaultSubLayer: return SortHelper::Sort(l1.DefaultSublayer, l2.DefaultSublayer, asc);
			case ColumnType::Id: return SortHelper::Sort(l1.LayerId, l2.LayerId, asc);
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
	ui.UIEnable(ID_EDIT_PROPERTIES, m_List.GetSelectedCount() == 1);
}

int CLayersView::GetFilterCount(LayerInfo& layer) const {
	if (layer.FilterCount < 0)
		layer.FilterCount = m_Engine.GetFilterCount(layer.LayerKey);
	return layer.FilterCount;
}

int CLayersView::GetCalloutCount(LayerInfo& layer) const {
	if (layer.CalloutCount < 0)
		layer.CalloutCount = m_Engine.GetCalloutCount(layer.LayerKey);
	return layer.CalloutCount;
}

void CLayersView::OnStateChanged(HWND, int from, int to, UINT oldState, UINT newState) {
	if ((newState & LVIS_SELECTED) || (oldState & LVIS_SELECTED))
		UpdateUI();
}

bool CLayersView::OnDoubleClickList(HWND, int row, int col, POINT const& pt) {
	LRESULT result;
	return ProcessWindowMessage(m_hWnd, WM_COMMAND, ID_EDIT_PROPERTIES, 0, result, 1);
}
