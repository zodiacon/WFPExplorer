#include "pch.h"
#include "ProvidersView.h"
#include "StringHelper.h"
#include <SortHelper.h>
#include "resource.h"
#include "WFPHelper.h"
#include "ClipboardHelper.h"

CProvidersView::CProvidersView(IMainFrame* frame, WFPEngine& engine) : CGenericListViewBase(frame), m_Engine(engine) {
}

void CProvidersView::UpdateUI() const {
	auto& ui = Frame()->UI();
	auto selected = m_List.GetSelectedCount();
	ui.UIEnable(ID_EDIT_PROPERTIES, selected == 1);
	ui.UIEnable(ID_EDIT_COPY, selected > 0);
	ui.UIEnable(ID_EDIT_DELETE, selected == 1);
}

LRESULT CProvidersView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	m_hWndClient = m_List.Create(m_hWnd, rcDefault, nullptr,
		WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | LVS_OWNERDATA | LVS_REPORT | LVS_SHOWSELALWAYS);
	m_List.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"Provider Key", 0, 300, ColumnType::Key, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Provider Name", 0, 330, ColumnType::Name);
	cm->AddColumn(L"Service Name", 0, 120, ColumnType::ServiceName);
	cm->AddColumn(L"Flags", 0, 120, ColumnType::Flags);
	cm->AddColumn(L"Provider Data", LVCFMT_RIGHT, 90, ColumnType::ProviderData);
	cm->AddColumn(L"Description", 0, 450, ColumnType::Desc);

	CImageList images;
	images.Create(16, 16, ILC_COLOR32 | ILC_MASK, 2, 2);
	images.AddIcon(AtlLoadIconImage(IDI_PROVIDER, 0, 16, 16));
	images.AddIcon(AtlLoadIconImage(IDI_PROVIDER_PERSISTENT, 0, 16, 16));
	m_List.SetImageList(images, LVSIL_SMALL);

	Refresh();

	return 0;
}

LRESULT CProvidersView::OnActivate(UINT, WPARAM active, LPARAM, BOOL&) const {
	if (active)
		UpdateUI();
	return 0;
}

void CProvidersView::OnStateChanged(HWND, int from, int to, UINT oldState, UINT newState) const {
	if ((newState & LVIS_SELECTED) || (oldState & LVIS_SELECTED))
		UpdateUI();
}

CString CProvidersView::GetDefaultSaveFile() const {
	return L"providers.csv";
}

bool CProvidersView::OnDoubleClickList(HWND, int row, int col, POINT const& pt) {
	LRESULT result;
	return ProcessWindowMessage(m_hWnd, WM_COMMAND, ID_EDIT_PROPERTIES, 0, result, 1);
}

LRESULT CProvidersView::OnRefresh(WORD, WORD, HWND, BOOL&) {
	Refresh();
	return 0;
}

LRESULT CProvidersView::OnProperties(WORD, WORD, HWND, BOOL&) {
	auto count = m_List.GetSelectedCount();
	ATLASSERT(count == 1);
	auto& provider = m_Providers[m_List.GetNextItem(-1, LVIS_SELECTED)];
	WFPHelper::ShowProviderProperties(m_Engine, provider.Data);

	return 0;
}

void CProvidersView::Refresh() {
	WFPProviderEnumerator penum(m_Engine.Handle());
	m_Providers = penum.Next<ProviderInfo>(256);
	Sort(m_List);
	m_List.SetItemCountEx((int)m_Providers.size(), LVSICF_NOSCROLL);
	Frame()->SetStatusText(2, std::format(L"{} Providers", m_Providers.size()).c_str());
}

CString CProvidersView::GetColumnText(HWND, int row, int col) {
	auto& pi = m_Providers[row];
	auto info = pi.Data;
	switch (GetColumnManager(m_List)->GetColumnTag<ColumnType>(col)) {
		case ColumnType::Key: return StringHelper::GuidToString(info->providerKey);
		case ColumnType::Name: return pi.Name();
		case ColumnType::Desc: return pi.Desc();
		case ColumnType::ProviderData: return info->providerData.size == 0 ? L"" : std::format(L"{} Bytes", info->providerData.size).c_str();
		case ColumnType::Flags:
			if (info->flags == 0)
				return L"0";
			return std::format(L"0x{:X} ({})", info->flags, (PCWSTR)StringHelper::WFPProviderFlagsToString(info->flags)).c_str();
		case ColumnType::ServiceName: return info->serviceName;
	}
	return CString();
}

void CProvidersView::DoSort(SortInfo const* si) {
	auto col = GetColumnManager(m_List)->GetColumnTag<ColumnType>(si->SortColumn);
	auto asc = si->SortAscending;

	auto compare = [&](auto& pr1, auto& pr2) {
		auto p1 = pr1.Data, p2 = pr2.Data;
		switch (col) {
			case ColumnType::Key: return SortHelper::Sort(StringHelper::GuidToString(p1->providerKey), StringHelper::GuidToString(p2->providerKey), asc);
			case ColumnType::Name: return SortHelper::Sort(pr1.Name(), pr2.Name(), asc);
			case ColumnType::Desc: return SortHelper::Sort(pr1.Desc(), pr2.Desc(), asc);
			case ColumnType::Flags: return SortHelper::Sort(p1->flags, p2->flags, asc);
			case ColumnType::ServiceName: return SortHelper::Sort(p1->serviceName, p2->serviceName, asc);
			case ColumnType::ProviderData: return SortHelper::Sort(p1->providerData.size, p2->providerData.size, asc);
		}
		return false;
	};
	std::ranges::sort(m_Providers, compare);
}

int CProvidersView::GetSaveColumnRange(HWND, int&) const {
	return 1;
}

int CProvidersView::GetRowImage(HWND, int row, int col) const {
	return m_Providers[row].Data->flags & FWPM_PROVIDER_FLAG_PERSISTENT ? 1 : 0;
}

CString const& CProvidersView::ProviderInfo::Name() const {
	if (m_Name.IsEmpty())
		m_Name = StringHelper::ParseMUIString(Data->displayData.name);
	return m_Name;
}

CString const& CProvidersView::ProviderInfo::Desc() const {
	if (m_Desc.IsEmpty())
		m_Desc = StringHelper::ParseMUIString(Data->displayData.description);
	return m_Desc;
}

LRESULT CProvidersView::OnCopy(WORD, WORD, HWND, BOOL&) {
	auto text = ListViewHelper::GetSelectedRowsAsString(m_List, L",");
	ClipboardHelper::CopyText(m_hWnd, text);

	return 0;
}

bool CProvidersView::OnRightClickList(HWND, int row, int col, POINT const& pt) const {
	if (row < 0)
		return false;

	CMenu menu;
	menu.LoadMenu(IDR_CONTEXT);

	return Frame()->TrackPopupMenu(menu.GetSubMenu(0), 0, pt.x, pt.y);
}

LRESULT CProvidersView::OnDelete(WORD, WORD, HWND, BOOL&) {
	auto count = m_List.GetSelectedCount();
	ATLASSERT(count == 1);
	auto& provider = m_Providers[m_List.GetNextItem(-1, LVIS_SELECTED)];
	if (!m_Engine.DeleteProvider(provider.Data->providerKey)) {
		AtlMessageBox(m_hWnd, L"Failed to delete provider", IDS_TITLE, MB_ICONERROR);
	}
	else {
		Refresh();
	}
	return 0;
}
