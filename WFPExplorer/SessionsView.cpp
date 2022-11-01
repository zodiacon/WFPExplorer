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

CSessionsView::CSessionsView(IMainFrame* frame, WFPEngine& engine) : CFrameView(frame), m_Engine(engine) {
}

BOOL CSessionsView::PreTranslateMessage(MSG* pMsg) {
	pMsg;
	return FALSE;
}

void CSessionsView::Refresh() {
	m_Sessions = m_Engine.EnumSessions<SessionInfo>();
	m_List.SetItemCountEx((int)m_Sessions.size(), LVSICF_NOSCROLL);
}

CString CSessionsView::GetColumnText(HWND, int row, int col) {
	auto& session = m_Sessions[row];
	switch (GetColumnManager(m_List)->GetColumnTag<ColumnType>(col)) {
		case ColumnType::Key: return StringHelper::GuidToString(session.SessionKey);
		case ColumnType::Name: return session.Name.c_str();
		case ColumnType::Desc: return session.Desc.c_str();
		case ColumnType::ProcessId: return std::to_wstring(session.ProcessId).c_str();
		case ColumnType::Flags: return std::to_wstring(session.Flags).c_str();
		case ColumnType::ProcessName:
			if (session.ProcessName.IsEmpty())
				session.ProcessName = ProcessHelper::GetProcessName(session.ProcessId);
			return session.ProcessName;
	}
	return CString();
}

void CSessionsView::DoSort(SortInfo const* si) {
	auto col = GetColumnManager(m_List)->GetColumnTag<ColumnType>(si->SortColumn);
	auto asc = si->SortAscending;

	auto compare = [&](auto& s1, auto& s2) {
		switch (col) {
			case ColumnType::Key: return SortHelper::Sort(StringHelper::GuidToString(s1.SessionKey), StringHelper::GuidToString(s2.SessionKey), asc);
			case ColumnType::Name: return SortHelper::Sort(s1.Name, s2.Name, asc);
			case ColumnType::Desc: return SortHelper::Sort(s1.Desc, s2.Desc, asc);
			case ColumnType::Flags: return SortHelper::Sort(s1.Flags, s2.Flags, asc);
			case ColumnType::ProcessId: return SortHelper::Sort(s1.ProcessId, s2.ProcessId, asc);
			case ColumnType::ProcessName: return SortHelper::Sort(s1.ProcessName, s2.ProcessName, asc);
		}
		return false;
	};
	std::ranges::sort(m_Sessions, compare);
}

LRESULT CSessionsView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	m_hWndClient = m_List.Create(m_hWnd, rcDefault, nullptr,
		WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | LVS_OWNERDATA | LVS_REPORT | LVS_SHOWSELALWAYS);
	m_List.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"Session Key", 0, 250, ColumnType::Key);
	cm->AddColumn(L"PID", LVCFMT_RIGHT, 90, ColumnType::ProcessId);
	cm->AddColumn(L"Process Name", LVCFMT_LEFT, 180, ColumnType::ProcessName);
	cm->AddColumn(L"Flags", LVCFMT_RIGHT, 80, ColumnType::Flags);
	cm->AddColumn(L"Session Name", 0, 180, ColumnType::Name);
	cm->AddColumn(L"Description", 0, 180, ColumnType::Desc);

	Refresh();

	return 0;
}

LRESULT CSessionsView::OnRefresh(WORD, WORD, HWND, BOOL&) {
	Refresh();
	return 0;
}
