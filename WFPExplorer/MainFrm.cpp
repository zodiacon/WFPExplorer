// MainFrm.cpp : implementation of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "resource.h"
#include "AboutDlg.h"
#include "SessionsView.h"
#include "FiltersView.h"
#include "ProvidersView.h"
#include "LayersView.h"
#include "SublayersView.h"
#include "CalloutsView.h"
#include "MainFrm.h"
#include <ToolbarHelper.h>

const int WINDOW_MENU_POSITION = 4;

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg) {
	if (CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
		return TRUE;

	return m_view.PreTranslateMessage(pMsg);
}

BOOL CMainFrame::OnIdle() {
	UIUpdateToolBar();
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
	UINT icons[] = { IDI_SESSION, IDI_FILTER, IDI_PROVIDER, IDI_LAYERS, IDI_SUBLAYER, IDI_CALLOUT };
	for (auto icon : icons)
		images.AddIcon(AtlLoadIconImage(icon, 0, 16, 16));
	m_view.SetImageList(images);

	UISetCheck(ID_VIEW_STATUS_BAR, 1);

	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	CMenuHandle menuMain = GetMenu();
	m_view.SetWindowMenu(menuMain.GetSubMenu(WINDOW_MENU_POSITION));

	if (!m_Engine.Open()) {
		AtlMessageBox(nullptr, L"Failed to open WFP Engine", IDR_MAINFRAME, MB_ICONERROR);
	}
	InitMenu();
	UIAddMenu(menuMain);
	AddMenu(menuMain);
	UpdateUI();

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

void CMainFrame::UpdateUI() {
	bool anyPage = m_view.GetPageCount() > 0;
	UIEnable(ID_FILE_SAVE, anyPage);
	UIEnable(ID_EDIT_COPY, anyPage);
	UIEnable(ID_EDIT_CUT, anyPage);
	UIEnable(ID_EDIT_PASTE, anyPage);
	UIEnable(ID_EDIT_DELETE, anyPage);
	UIEnable(ID_VIEW_REFRESH, anyPage);
}

LRESULT CMainFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);

	bHandled = FALSE;
	return 1;
}

LRESULT CMainFrame::OnRebuildWindowMenu(UINT, WPARAM, LPARAM, BOOL& bHandled) {
	AddSubMenu(GetSubMenu(GetMenu(), WINDOW_MENU_POSITION));

	return 0;
}

LRESULT CMainFrame::OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	PostMessage(WM_CLOSE);
	return 0;
}

LRESULT CMainFrame::OnViewSessions(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	auto view = new CSessionsView(this, m_Engine);
	view->Create(m_view, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);
	m_view.AddPage(view->m_hWnd, L"Sessions", 0, view);
	AddCommand(ID_WINDOW_TABFIRST + m_view.GetPageCount() - 1, IDI_SESSION);

	return 0;
}

LRESULT CMainFrame::OnViewFilters(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	auto view = new CFiltersView(this, m_Engine);
	view->Create(m_view, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);
	m_view.AddPage(view->m_hWnd, L"Filters", 1, view);
	AddCommand(ID_WINDOW_TABFIRST + m_view.GetPageCount() - 1, IDI_FILTER);

	return 0;
}

LRESULT CMainFrame::OnViewProviders(WORD, WORD, HWND, BOOL&) {
	auto view = new CProvidersView(this, m_Engine);
	view->Create(m_view, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);
	m_view.AddPage(view->m_hWnd, L"Providers", 2, view);
	AddCommand(ID_WINDOW_TABFIRST + m_view.GetPageCount() - 1, IDI_PROVIDER);

	return 0;
}

LRESULT CMainFrame::OnViewLayers(WORD, WORD, HWND, BOOL&) {
	auto view = new CLayersView(this, m_Engine);
	view->Create(m_view, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);
	m_view.AddPage(view->m_hWnd, L"Layers", 3, view);
	AddCommand(ID_WINDOW_TABFIRST + m_view.GetPageCount() - 1, IDI_LAYERS);

	return 0;
}

LRESULT CMainFrame::OnViewSublayers(WORD, WORD, HWND, BOOL&) {
	auto view = new CSublayersView(this, m_Engine);
	view->Create(m_view, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);
	m_view.AddPage(view->m_hWnd, L"Sublayers", 4, view);
	AddCommand(ID_WINDOW_TABFIRST + m_view.GetPageCount() - 1, IDI_SUBLAYER);

	return 0;
}

LRESULT CMainFrame::OnViewCallouts(WORD, WORD, HWND, BOOL&) {
	auto view = new CCalloutsView(this, m_Engine);
	view->Create(m_view, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);
	m_view.AddPage(view->m_hWnd, L"Callouts", 5, view);
	AddCommand(ID_WINDOW_TABFIRST + m_view.GetPageCount() - 1, IDI_CALLOUT);

	return 0;
}

LRESULT CMainFrame::OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	auto bVisible = !::IsWindowVisible(m_hWndStatusBar);
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

LRESULT CMainFrame::OnPageActivated(int, LPNMHDR, BOOL&) {
	int page = m_view.GetActivePage();
	bool handled = false;
	if (page >= 0) {
		handled = ::SendMessage(m_view.GetPageHWND(page), WM_ACTIVATE, 1, 0);
	}
	if(!handled) {
		UpdateUI();
	}
	return 0;
}
