#include "pch.h"
#include "SublayersView.h"
#include "StringHelper.h"
#include <SortHelper.h>
#include "resource.h"

CSublayersView::CSublayersView(IMainFrame* frame, WFPEngine& engine) : CFrameView(frame), m_Engine(engine) {
}

LRESULT CSublayersView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	m_hWndClient = m_List.Create(m_hWnd, rcDefault, nullptr,
		WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | LVS_OWNERDATA | LVS_REPORT | LVS_SHOWSELALWAYS);
	m_List.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"Layer Key", 0, 250, ColumnType::Key);
	cm->AddColumn(L"Name", 0, 180, ColumnType::Name);
	cm->AddColumn(L"Flags", 0, 120, ColumnType::Flags);
	cm->AddColumn(L"Weight", LVCFMT_RIGHT, 80, ColumnType::Weight);
	cm->AddColumn(L"Provider", 0, 180, ColumnType::Provider);
	cm->AddColumn(L"Description", 0, 180, ColumnType::Desc);

	CImageList images;
	images.Create(16, 16, ILC_COLOR32 | ILC_MASK, 1, 1);
	images.AddIcon(AtlLoadIconImage(IDI_SUBLAYER, 0, 16, 16));
	m_List.SetImageList(images, LVSIL_SMALL);

	Refresh();

	return 0;
}

LRESULT CSublayersView::OnRefresh(WORD, WORD, HWND, BOOL&) {
	Refresh();
	return 0;
}

void CSublayersView::Refresh() {
	m_Layers = m_Engine.EnumSubLayers<SubLayerInfo>();
	m_List.SetItemCountEx((int)m_Layers.size(), LVSICF_NOSCROLL);
	Frame()->SetStatusText(6, std::format(L"{} Sublayers", m_Layers.size()).c_str());
}

CString CSublayersView::GetColumnText(HWND, int row, int col) {
	auto& info = m_Layers[row];
	switch (GetColumnManager(m_List)->GetColumnTag<ColumnType>(col)) {
		case ColumnType::Key: return StringHelper::GuidToString(info.SubLayerKey);
		case ColumnType::Name: return info.Name.c_str();
		case ColumnType::Desc: return info.Desc.c_str();
		case ColumnType::Flags: 
			if (info.Flags == WFPSubLayerFlags::None)
				return L"0";
			return std::format(L"0x{:X} ({})", (UINT32)info.Flags,
				(PCWSTR)StringHelper::WFPSubLayerFlagsToString(info.Flags)).c_str();

		case ColumnType::Weight: return std::to_wstring(info.Weight).c_str();
		case ColumnType::Provider:
			if (info.ProviderName.IsEmpty()) {
				if (info.ProviderKey != GUID_NULL) {
					auto provider = m_Engine.GetProviderByKey(info.ProviderKey);
					if (provider && !provider.value().Name.empty() && provider.value().Name[0] != L'@')
						info.ProviderName = provider.value().Name.c_str();
					else
						info.ProviderName = StringHelper::GuidToString(info.ProviderKey);
				}
			}
			return info.ProviderName;

	}
	return CString();
}

void CSublayersView::DoSort(SortInfo const* si) {
	auto col = GetColumnManager(m_List)->GetColumnTag<ColumnType>(si->SortColumn);
	auto asc = si->SortAscending;

	auto compare = [&](auto& l1, auto& l2) {
		switch (col) {
			case ColumnType::Key: return SortHelper::Sort(StringHelper::GuidToString(l1.SubLayerKey), StringHelper::GuidToString(l2.SubLayerKey), asc);
			case ColumnType::Name: return SortHelper::Sort(l1.Name, l2.Name, asc);
			case ColumnType::Desc: return SortHelper::Sort(l1.Desc, l2.Desc, asc);
			case ColumnType::Flags: return SortHelper::Sort(l1.Flags, l2.Flags, asc);
			case ColumnType::Provider: return SortHelper::Sort(l1.ProviderName, l2.ProviderName, asc);
			case ColumnType::Weight: return SortHelper::Sort(l1.Weight, l2.Weight, asc);
		}
		return false;
	};
	std::ranges::sort(m_Layers, compare);
}

int CSublayersView::GetSaveColumnRange(HWND, int&) const {
	return 1;
}

int CSublayersView::GetRowImage(HWND, int row, int col) const {
	return 0;
}
