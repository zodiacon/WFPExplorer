// MainFrm.cpp : implementation of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "resource.h"
#include "AboutDlg.h"
#include "SessionsView.h"
#include "FiltersView.h"
#include "ProvidersView.h"
#include "MainFrm.h"
#include <ToolbarHelper.h>

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg) {
	if (CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
		return TRUE;

	return m_view.PreTranslateMessage(pMsg);
}

BOOL CMainFrame::OnIdle() {
	return FALSE;
}

LRESULT CMainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	if (!m_Engine.Open()) {
		AtlMessageBox(nullptr, L"Failed to open WFP Engine", IDR_MAINFRAME, MB_ICONERROR);
		return -1;
	}

	CreateSimpleStatusBar();

	ToolBarButtonInfo const buttons[] = {
		{ ID_VIEW_REFRESH, IDI_REFRESH },
		{ 0 },
		{ ID_VIEW_SESSIONS, IDI_SESSION },
		{ ID_VIEW_PROVIDERS, IDI_PROVIDER },
		{ ID_VIEW_FILTERS, IDI_FILTER },
		{ ID_VIEW_LAYERS, IDI_LAYERS },
		{ ID_VIEW_SUBLAYERS, IDI_SUBLAYER },
		{ ID_VIEW_CALLOUTS, IDI_CALLOUT },
	};
	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	auto tb = ToolbarHelper::CreateAndInitToolBar(m_hWnd, buttons, _countof(buttons));
	AddSimpleReBarBand(tb);
	UIAddToolBar(tb);

	m_view.m_bTabCloseButton = false;
	m_hWndClient = m_view.Create(m_hWnd, rcDefault, nullptr, 
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);
	CImageList images;
	images.Create(16, 16, ILC_COLOR32, 8, 4);
	UINT icons[] = { IDI_SESSION, IDI_FILTER, IDI_PROVIDER, IDI_LAYERS, IDI_CALLOUT };
	for (auto icon : icons)
		images.AddIcon(AtlLoadIconImage(icon, 0, 16, 16));
	m_view.SetImageList(images);

	UISetCheck(ID_VIEW_STATUS_BAR, 1);

	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	const int WINDOW_MENU_POSITION = 4;

	CMenuHandle menuMain = GetMenu();
	m_view.SetWindowMenu(menuMain.GetSubMenu(WINDOW_MENU_POSITION));

	if (!m_Engine.Open()) {
		AtlMessageBox(nullptr, L"Failed to open WFP Engine", IDR_MAINFRAME, MB_ICONERROR);
	}
	InitMenu();
	UIAddMenu(menuMain);
	AddMenu(menuMain);

	return 0;
}

void CMainFrame::InitMenu() {
	struct {
		UINT id, icon;
		HICON hIcon = nullptr;
	} cmds[] = {
		{ ID_EDIT_COPY, IDI_COPY },
		{ ID_EDIT_CUT, IDI_CUT },
		{ ID_EDIT_PASTE, IDI_PASTE },
		{ ID_VIEW_SESSIONS, IDI_SESSION },
		{ ID_VIEW_FILTERS, IDI_FILTER },
		{ ID_VIEW_CALLOUTS, IDI_CALLOUT },
		{ ID_VIEW_REFRESH, IDI_REFRESH },
		{ ID_VIEW_PROVIDERS, IDI_PROVIDER },
		{ ID_VIEW_LAYERS, IDI_LAYERS },
		{ ID_VIEW_SUBLAYERS, IDI_SUBLAYER },
		{ ID_OPTIONS_ALWAYSONTOP, IDI_PIN },
		{ ID_EDIT_DELETE, IDI_DELETE },
	};
	for (auto& cmd : cmds) {
		if (cmd.icon)
			AddCommand(cmd.id, cmd.icon);
		else
			AddCommand(cmd.id, cmd.hIcon);
	}
}

LRESULT CMainFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);

	bHandled = FALSE;
	return 1;
}

LRESULT CMainFrame::OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	PostMessage(WM_CLOSE);
	return 0;
}

LRESULT CMainFrame::OnViewSessions(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	auto view = new CSessionsView(this, m_Engine);
	view->Create(m_view, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);
	m_view.AddPage(view->m_hWnd, L"Sessions", 0, view);

	return 0;
}

LRESULT CMainFrame::OnViewFilters(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	auto view = new CFiltersView(this, m_Engine);
	view->Create(m_view, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);
	m_view.AddPage(view->m_hWnd, L"Filters", 1, view);

	return 0;
}

LRESULT CMainFrame::OnViewProviders(WORD, WORD, HWND, BOOL&) {
	auto view = new CProvidersView(this, m_Engine);
	view->Create(m_view, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);
	m_view.AddPage(view->m_hWnd, L"Providers", 2, view);

	return 0;
}

LRESULT CMainFrame::OnViewLayers(WORD, WORD, HWND, BOOL&) {
	return LRESULT();
}

LRESULT CMainFrame::OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	BOOL bVisible = !::IsWindowVisible(m_hWndStatusBar);
	::ShowWindow(m_hWndStatusBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
	UISetCheck(ID_VIEW_STATUS_BAR, bVisible);
	UpdateLayout();
	return 0;
}

LRESULT CMainFrame::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	CAboutDlg dlg;
	dlg.DoModal();
	return 0;
}

LRESULT CMainFrame::OnWindowClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int nActivePage = m_view.GetActivePage();
	if (nActivePage != -1)
		m_view.RemovePage(nActivePage);
	else
		::MessageBeep((UINT)-1);

	return 0;
}

LRESULT CMainFrame::OnWindowCloseAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	m_view.RemoveAllPages();

	return 0;
}

LRESULT CMainFrame::OnWindowActivate(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int nPage = wID - ID_WINDOW_TABFIRST;
	m_view.SetActivePage(nPage);

	return 0;
}
