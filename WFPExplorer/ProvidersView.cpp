#include "pch.h"
#include "ProvidersView.h"
#include "StringHelper.h"
#include <SortHelper.h>

CProvidersView::CProvidersView(IMainFrame* frame, WFPEngine& engine) : CFrameView(frame), m_Engine(engine) {
}

LRESULT CProvidersView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	m_hWndClient = m_List.Create(m_hWnd, rcDefault, nullptr,
		WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | LVS_OWNERDATA | LVS_REPORT | LVS_SHOWSELALWAYS);
	m_List.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"Provider Key", 0, 250, ColumnType::Key);
	cm->AddColumn(L"Service Name", 0, 180, ColumnType::ServiceName);
	cm->AddColumn(L"Provider Name", 0, 180, ColumnType::Name);
	cm->AddColumn(L"Description", 0, 180, ColumnType::Desc);
	cm->AddColumn(L"Flags", LVCFMT_RIGHT, 80, ColumnType::Flags);

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
}

CString CProvidersView::GetColumnText(HWND, int row, int col) {
	auto& info = m_Providers[row];
	switch (GetColumnManager(m_List)->GetColumnTag<ColumnType>(col)) {
		case ColumnType::Key: return StringHelper::GuidToString(info.ProviderKey);
		case ColumnType::Name: return info.Name.c_str();
		case ColumnType::Desc: return info.Desc.c_str();
		case ColumnType::Flags: return std::format(L"0x{:X}", info.Flags).c_str();
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
		}
		return false;
	};
	std::ranges::sort(m_Providers, compare);
}

int CProvidersView::GetSaveColumnRange(HWND, int&) const {
	return 1;
}
