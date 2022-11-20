#include "pch.h"
#include "ProvidersView.h"
#include "StringHelper.h"
#include <SortHelper.h>
#include "resource.h"

CProvidersView::CProvidersView(IMainFrame* frame, WFPEngine& engine) : CFrameView(frame), m_Engine(engine) {
}

LRESULT CProvidersView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	m_hWndClient = m_List.Create(m_hWnd, rcDefault, nullptr,
		WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | LVS_OWNERDATA | LVS_REPORT | LVS_SHOWSELALWAYS);
	m_List.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"Provider Key", 0, 280, ColumnType::Key);
	cm->AddColumn(L"Provider Name", 0, 220, ColumnType::Name);
	cm->AddColumn(L"Service Name", 0, 180, ColumnType::ServiceName);
	cm->AddColumn(L"Flags", 0, 120, ColumnType::Flags);
	cm->AddColumn(L"Provider Data", LVCFMT_RIGHT, 90, ColumnType::ProviderData);
	cm->AddColumn(L"Description", 0, 250, ColumnType::Desc);

	CImageList images;
	images.Create(16, 16, ILC_COLOR32 | ILC_MASK, 2, 2);
	images.AddIcon(AtlLoadIconImage(IDI_PROVIDER, 0, 16, 16));
	images.AddIcon(AtlLoadIconImage(IDI_PROVIDER_PERSISTENT, 0, 16, 16));
	m_List.SetImageList(images, LVSIL_SMALL);

	Refresh();

	return 0;
}

LRESULT CProvidersView::OnRefresh(WORD, WORD, HWND, BOOL&) {
	Refresh();
	return 0;
}

void CProvidersView::Refresh() {
	m_Providers = m_Engine.EnumProviders();
	m_List.SetItemCountEx((int)m_Providers.size(), LVSICF_NOSCROLL);
	Frame()->SetStatusText(2, std::format(L"{} Providers", m_Providers.size()).c_str());
}

CString CProvidersView::GetColumnText(HWND, int row, int col) {
	auto& info = m_Providers[row];
	switch (GetColumnManager(m_List)->GetColumnTag<ColumnType>(col)) {
		case ColumnType::Key: return StringHelper::GuidToString(info.ProviderKey);
		case ColumnType::Name: return info.Name.c_str();
		case ColumnType::Desc: return info.Desc.c_str();
		case ColumnType::ProviderData: return info.ProviderDataSize == 0 ? L"" : std::format(L"{} Bytes", info.ProviderDataSize).c_str();
		case ColumnType::Flags:
			if (info.Flags == WFPProviderFlags::None)
				return L"0";
			return std::format(L"0x{:X} ({})", (UINT32)info.Flags, StringHelper::WFPProviderFlagsToString(info.Flags)).c_str();
		case ColumnType::ServiceName: return info.ServiceName.c_str();
	}
	return CString();
}

void CProvidersView::DoSort(SortInfo const* si) {
	auto col = GetColumnManager(m_List)->GetColumnTag<ColumnType>(si->SortColumn);
	auto asc = si->SortAscending;

	auto compare = [&](auto& p1, auto& p2) {
		switch (col) {
			case ColumnType::Key: return SortHelper::Sort(StringHelper::GuidToString(p1.ProviderKey), StringHelper::GuidToString(p2.ProviderKey), asc);
			case ColumnType::Name: return SortHelper::Sort(p1.Name, p2.Name, asc);
			case ColumnType::Desc: return SortHelper::Sort(p1.Desc, p2.Desc, asc);
			case ColumnType::Flags: return SortHelper::Sort(p1.Flags, p2.Flags, asc);
			case ColumnType::ServiceName: return SortHelper::Sort(p1.ServiceName, p2.ServiceName, asc);
			case ColumnType::ProviderData: return SortHelper::Sort(p1.ProviderDataSize, p2.ProviderDataSize, asc);
		}
		return false;
	};
	std::ranges::sort(m_Providers, compare);
}

int CProvidersView::GetSaveColumnRange(HWND, int&) const {
	return 1;
}

int CProvidersView::GetRowImage(HWND, int row, int col) const {
	return (m_Providers[row].Flags & WFPProviderFlags::Persistent) == WFPProviderFlags::Persistent ? 1 : 0;
}
