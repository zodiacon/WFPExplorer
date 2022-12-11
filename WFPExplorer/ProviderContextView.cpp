#include "pch.h"
#include "ProviderContextView.h"
#include "StringHelper.h"
#include <SortHelper.h>
#include "resource.h"

CProviderContextView::CProviderContextView(IMainFrame* frame, WFPEngine& engine) : CFrameView(frame), m_Engine(engine) {
}

LRESULT CProviderContextView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	m_hWndClient = m_List.Create(m_hWnd, rcDefault, nullptr,
		WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | LVS_OWNERDATA | LVS_REPORT | LVS_SHOWSELALWAYS);
	m_List.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"Context Key", 0, 260, ColumnType::Key);
	cm->AddColumn(L"Type", 0, 150, ColumnType::Type);
	cm->AddColumn(L"Flags", 0, 100, ColumnType::Flags);
	cm->AddColumn(L"Data Size", LVCFMT_RIGHT, 90, ColumnType::DataSize);
	cm->AddColumn(L"Context ID", 0, 120, ColumnType::Id);
	cm->AddColumn(L"Provider", 0, 240, ColumnType::Provider);
	cm->AddColumn(L"Name", 0, 220, ColumnType::Name);
	cm->AddColumn(L"Description", 0, 300, ColumnType::Desc);

	CImageList images;
	images.Create(16, 16, ILC_COLOR32 | ILC_MASK, 2, 2);
	images.AddIcon(AtlLoadIconImage(IDI_CONTEXT, 0, 16, 16));
	//images.AddIcon(AtlLoadIconImage(IDI_PROVIDER_PERSISTENT, 0, 16, 16));
	m_List.SetImageList(images, LVSIL_SMALL);

	Refresh();

	return 0;
}

LRESULT CProviderContextView::OnRefresh(WORD, WORD, HWND, BOOL&) {
	Refresh();
	return 0;
}

void CProviderContextView::Refresh() {
	m_Contexts = m_Engine.EnumProviderContexts();
	m_List.SetItemCountEx((int)m_Contexts.size(), LVSICF_NOSCROLL);
}

CString CProviderContextView::GetColumnText(HWND, int row, int col) {
	auto& info = m_Contexts[row];
	switch (GetColumnManager(m_List)->GetColumnTag<ColumnType>(col)) {
		case ColumnType::Key: return StringHelper::GuidToString(info.ProviderKey);
		case ColumnType::Name: return info.Name.c_str();
		case ColumnType::Type: return std::format(L"{} ({})", StringHelper::WFPProviderContextTypeToString(info.Type), (int)info.Type).c_str();
		case ColumnType::Provider:
			if (info.ProviderKey != GUID_NULL) {
				auto provider = m_Engine.GetProviderByKey(info.ProviderKey);
				if (provider && provider->displayData.name && provider->displayData.name[0] != L'@')
					return provider->displayData.name;
				return StringHelper::GuidToString(info.ProviderKey);
			}
			break;
		case ColumnType::Desc: return info.Desc.c_str();
		case ColumnType::Id: return std::format(L"0x{:X}", info.ProviderContextId).c_str();
		case ColumnType::DataSize: return std::format(L"{}", info.ProviderDataSize).c_str();
		case ColumnType::Flags:
			if (info.Flags == WFPProviderContextFlags::None)
				return L"0";
			return std::format(L"0x{:X} ({})", (UINT32)info.Flags, StringHelper::WFPProviderContextFlagsToString(info.Flags)).c_str();
	}
	return CString();
}

void CProviderContextView::DoSort(SortInfo const* si) {
	auto col = GetColumnManager(m_List)->GetColumnTag<ColumnType>(si->SortColumn);
	auto asc = si->SortAscending;

	auto compare = [&](auto& p1, auto& p2) {
		switch (col) {
			case ColumnType::Key: return SortHelper::Sort(StringHelper::GuidToString(p1.ProviderKey), StringHelper::GuidToString(p2.ProviderKey), asc);
			case ColumnType::Name: return SortHelper::Sort(p1.Name, p2.Name, asc);
			case ColumnType::Desc: return SortHelper::Sort(p1.Desc, p2.Desc, asc);
			case ColumnType::Flags: return SortHelper::Sort(p1.Flags, p2.Flags, asc);
			case ColumnType::Type: return SortHelper::Sort(p1.Type, p2.Type, asc);
			case ColumnType::Id: return SortHelper::Sort(p1.ProviderContextId, p2.ProviderContextId, asc);
			case ColumnType::DataSize: return SortHelper::Sort(p1.ProviderDataSize, p2.ProviderDataSize, asc);
			case ColumnType::Provider: return SortHelper::Sort(StringHelper::GuidToString(p1.ProviderKey), StringHelper::GuidToString(p2.ProviderKey), asc);
		}
		return false;
	};
	std::ranges::sort(m_Contexts, compare);
}

int CProviderContextView::GetSaveColumnRange(HWND, int&) const {
	return 1;
}

int CProviderContextView::GetRowImage(HWND, int row, int col) const {
	return 0;
}
