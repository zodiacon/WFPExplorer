#include "pch.h"
#include "LayersView.h"
#include "StringHelper.h"
#include <SortHelper.h>
#include "resource.h"

CLayersView::CLayersView(IMainFrame* frame, WFPEngine& engine) : CFrameView(frame), m_Engine(engine) {
}

LRESULT CLayersView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	m_hWndClient = m_List.Create(m_hWnd, rcDefault, nullptr,
		WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | LVS_OWNERDATA | LVS_REPORT | LVS_SHOWSELALWAYS);
	m_List.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"Layer Key", 0, 250, ColumnType::Key);
	cm->AddColumn(L"Layer ID", LVCFMT_RIGHT, 90, ColumnType::Id);
	cm->AddColumn(L"Name", 0, 180, ColumnType::Name);
	cm->AddColumn(L"Flags", LVCFMT_LEFT, 150, ColumnType::Flags);
	cm->AddColumn(L"Fields", LVCFMT_RIGHT, 80, ColumnType::Fields);
	cm->AddColumn(L"Default Sublayer", 0, 250, ColumnType::DefaultSubLayer);
	cm->AddColumn(L"Description", 0, 180, ColumnType::Desc);

	CImageList images;
	images.Create(16, 16, ILC_COLOR32 | ILC_MASK, 1, 1);
	images.AddIcon(AtlLoadIconImage(IDI_LAYERS, 0, 16, 16));
	m_List.SetImageList(images, LVSIL_SMALL);

	Refresh();

	return 0;
}

LRESULT CLayersView::OnRefresh(WORD, WORD, HWND, BOOL&) {
	Refresh();
	return 0;
}

void CLayersView::Refresh() {
	m_Layers = m_Engine.EnumLayers<LayerInfo>();
	m_List.SetItemCountEx((int)m_Layers.size(), LVSICF_NOSCROLL);
}

CString CLayersView::GetColumnText(HWND, int row, int col) {
	auto& info = m_Layers[row];
	switch (GetColumnManager(m_List)->GetColumnTag<ColumnType>(col)) {
		case ColumnType::Key: return StringHelper::GuidToString(info.LayerKey);
		case ColumnType::DefaultSubLayer:
			if (info.DefaultSublayer.IsEmpty()) {
				if (info.DefaultSubLayerKey != GUID_NULL) {
					auto sl = m_Engine.GetSubLayerByKey(info.DefaultSubLayerKey);
					if (sl && !sl.value().Name.empty())
						info.DefaultSublayer = sl.value().Name.c_str();
					else
						info.DefaultSublayer = StringHelper::GuidToString(info.DefaultSubLayerKey);
				}
			}
			return info.DefaultSublayer;

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
