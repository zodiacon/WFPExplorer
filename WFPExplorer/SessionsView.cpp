// View.cpp : implementation of the CSessionsView class
//
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "resource.h"
#include "SessionsView.h"
#include <WFPEngine.h>
#include "StringHelper.h"
#include "ProcessHelper.h"
#include <SortHelper.h>

CSessionsView::CSessionsView(IMainFrame* frame, WFPEngine& engine) : CGenericListViewBase(frame), m_Engine(engine) {
}

BOOL CSessionsView::PreTranslateMessage(MSG* pMsg) {
	pMsg;
	return FALSE;
}

void CSessionsView::Refresh() {
	WFPSessionEnumerator enumerator(m_Engine.Handle());
	m_Sessions = enumerator.Next<SessionInfo>();
	Sort(m_List);
	m_List.SetItemCountEx((int)m_Sessions.size(), LVSICF_NOSCROLL);
	Frame()->SetStatusText(1, std::format(L"{} Sessions", m_Sessions.size()).c_str());
}

CString CSessionsView::GetColumnText(HWND, int row, int col) {
	auto& session = m_Sessions[row];
	auto data = session.Data;
	switch (GetColumnManager(m_List)->GetColumnTag<ColumnType>(col)) {
		case ColumnType::Key: return StringHelper::GuidToString(data->sessionKey);
		case ColumnType::Name: return data->displayData.name;
		case ColumnType::Desc: return data->displayData.description;
		case ColumnType::SID: return StringHelper::FormatSID(data->sid);
		case ColumnType::KernelMode: return data->kernelMode ? L"Yes" : L"";
		case ColumnType::UserName: return data->username;
		case ColumnType::ProcessId: return std::to_wstring(data->processId).c_str();
		case ColumnType::Flags: 
			if (data->flags == 0)
				return L"0";
			return std::format(L"0x{:X} ({})", data->flags, (PCWSTR)StringHelper::WFPSessionFlagsToString(data->flags)).c_str();
		case ColumnType::ProcessName:
			if (session.ProcessName.IsEmpty())
				session.ProcessName = ProcessHelper::GetProcessName(data->processId);
			return session.ProcessName;
	}
	return CString();
}

void CSessionsView::DoSort(SortInfo const* si) {
	auto col = GetColumnManager(m_List)->GetColumnTag<ColumnType>(si->SortColumn);
	auto asc = si->SortAscending;

	auto compare = [&](auto& s1, auto& s2) {
		auto d1 = s1.Data, d2 = s2.Data;
		switch (col) {
			case ColumnType::Key: return SortHelper::Sort(StringHelper::GuidToString(d1->sessionKey), StringHelper::GuidToString(d2->sessionKey), asc);
			case ColumnType::Name: return SortHelper::Sort(d1->displayData.name, d2->displayData.name, asc);
			case ColumnType::Desc: return SortHelper::Sort(d1->displayData.description, d2->displayData.description, asc);
			case ColumnType::Flags: return SortHelper::Sort(d1->flags, d2->flags, asc);
			case ColumnType::ProcessId: return SortHelper::Sort(d1->processId, d2->processId, asc);
			case ColumnType::ProcessName: return SortHelper::Sort(s1.ProcessName, s2.ProcessName, asc);
			case ColumnType::UserName: return SortHelper::Sort(d1->username, d2->username, asc);
			case ColumnType::KernelMode: return SortHelper::Sort(d1->kernelMode, d2->kernelMode, asc);
		}
		return false;
	};
	std::ranges::sort(m_Sessions, compare);
}

int CSessionsView::GetRowImage(HWND, int row, int col) const {
	return m_Sessions[row].Data->flags & FWPM_SESSION_FLAG_DYNAMIC ? 1 : 0;
}

CString CSessionsView::GetDefaultSaveFile() const {
	return L"sessions.csv";
}

LRESULT CSessionsView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	m_hWndClient = m_List.Create(m_hWnd, rcDefault, nullptr,
		WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | LVS_OWNERDATA | LVS_REPORT | LVS_SHOWSELALWAYS);
	m_List.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"Session Key", 0, 300, ColumnType::Key, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"PID", LVCFMT_RIGHT, 90, ColumnType::ProcessId, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Process Name", LVCFMT_LEFT, 180, ColumnType::ProcessName);
	cm->AddColumn(L"User Name", LVCFMT_LEFT, 220, ColumnType::UserName);
	cm->AddColumn(L"Flags", LVCFMT_LEFT, 120, ColumnType::Flags);
	cm->AddColumn(L"Session Name", 0, 180, ColumnType::Name);
	cm->AddColumn(L"Kernel?", 0, 80, ColumnType::KernelMode);
	cm->AddColumn(L"Description", 0, 180, ColumnType::Desc);

	CImageList images;
	images.Create(16, 16, ILC_COLOR32 | ILC_MASK, 2, 2);
	images.AddIcon(AtlLoadIconImage(IDI_SESSION, 0, 16, 16));
	images.AddIcon(AtlLoadIconImage(IDI_SESSION_DYNAMIC, 0, 16, 16));
	m_List.SetImageList(images, LVSIL_SMALL);

	Refresh();

	return 0;
}

LRESULT CSessionsView::OnRefresh(WORD, WORD, HWND, BOOL&) {
	Refresh();
	return 0;
}
