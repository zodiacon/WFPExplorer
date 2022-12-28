#include "pch.h"
#include "ProviderContextView.h"
#include "StringHelper.h"
#include <SortHelper.h>
#include "resource.h"
#include <WFPEnumerators.h>

CProviderContextView::CProviderContextView(IMainFrame* frame, WFPEngine& engine) : CGenericListViewBase(frame), m_Engine(engine) {
}

CString CProviderContextView::GetProviderName(GUID const* key) const {
	CString name;
	if (key) {
		if (auto provider = m_Engine.GetProviderByKey(*key); provider)
			name = StringHelper::ParseMUIString(provider->displayData.name);
		if(name.IsEmpty() || name[0] == L'@')
			name =StringHelper::GuidToString(*key);
	}
	return name;
}

LRESULT CProviderContextView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	m_hWndClient = m_List.Create(m_hWnd, rcDefault, nullptr,
		WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | LVS_OWNERDATA | LVS_REPORT | LVS_SHOWSELALWAYS);
	m_List.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"Context Key", 0, 300, ColumnType::Key, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Type", 0, 150, ColumnType::Type);
	cm->AddColumn(L"Name", 0, 220, ColumnType::Name);
	cm->AddColumn(L"Flags", 0, 100, ColumnType::Flags);
	cm->AddColumn(L"Data Size", LVCFMT_RIGHT, 90, ColumnType::DataSize);
	cm->AddColumn(L"Context ID", 0, 150, ColumnType::Id, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Provider", 0, 240, ColumnType::Provider);
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
	m_Contexts = WFPProviderContextEnumerator(m_Engine.Handle()).Next(256);
	m_List.SetItemCountEx((int)m_Contexts.size(), LVSICF_NOSCROLL);
}

CString CProviderContextView::GetColumnText(HWND, int row, int col) {
	auto info = m_Contexts[row];
	switch (GetColumnManager(m_List)->GetColumnTag<ColumnType>(col)) {
		case ColumnType::Key: return StringHelper::GuidToString(info->providerContextKey);
		case ColumnType::Name: return StringHelper::ParseMUIString(info->displayData.name);
		case ColumnType::Type: return std::format(L"{} ({})", StringHelper::WFPProviderContextTypeToString(info->type), (UINT32)info->type).c_str();
		case ColumnType::Provider: return GetProviderName(info->providerKey);
		case ColumnType::Desc: return StringHelper::ParseMUIString(info->displayData.description);
		case ColumnType::Id: return std::format(L"0x{:X}", info->providerContextId).c_str();
		case ColumnType::DataSize: return info->providerData.size ? std::format(L"{} bytes", info->providerData.size).c_str() : L"";
		case ColumnType::Flags:
			if (info->flags == 0)
				return L"0";
			return std::format(L"0x{:X} ({})", info->flags, (PCWSTR)StringHelper::WFPProviderContextFlagsToString(info->flags)).c_str();
	}
	return CString();
}

void CProviderContextView::DoSort(SortInfo const* si) {
	auto col = GetColumnManager(m_List)->GetColumnTag<ColumnType>(si->SortColumn);
	auto asc = si->SortAscending;

	auto compare = [&](auto p1, auto p2) {
		switch (col) {
			case ColumnType::Key: return SortHelper::Sort(StringHelper::GuidToString(p1->providerContextKey), StringHelper::GuidToString(p2->providerContextKey), asc);
			case ColumnType::Name: return SortHelper::Sort(StringHelper::ParseMUIString(p1->displayData.name), StringHelper::ParseMUIString(p2->displayData.name), asc);
			case ColumnType::Desc: return SortHelper::Sort(StringHelper::ParseMUIString(p1->displayData.description), StringHelper::ParseMUIString(p2->displayData.description), asc);
			case ColumnType::Flags: return SortHelper::Sort(p1->flags, p2->flags, asc);
			case ColumnType::Type: return SortHelper::Sort(p1->type, p2->type, asc);
			case ColumnType::Id: return SortHelper::Sort(p1->providerContextId, p2->providerContextId, asc);
			case ColumnType::DataSize: return SortHelper::Sort(p1->providerData.size, p2->providerData.size, asc);
			case ColumnType::Provider: return SortHelper::Sort(GetProviderName(p1->providerKey), GetProviderName(p2->providerKey), asc);
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

CString CProviderContextView::GetDefaultSaveFile() const {
	return L"providercontexts.csv";
}
