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
#include "ProviderContextView.h"
#include "MainFrm.h"
#include <ToolbarHelper.h>
#include "AppSettings.h"
#include "HierarchyView.h"
#include "NetEventsView.h"

const int WINDOW_MENU_POSITION = 4;

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg) {
	if (m_pFindDlg && m_pFindDlg->IsDialogMessageW(pMsg))
		return TRUE;

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
	m_StatusBar.SubclassWindow(m_hWndStatusBar);
	int parts[] = { 120, 240, 360, 480, 600, 720, 840 };
	m_StatusBar.SetParts(_countof(parts), parts);
	m_StatusBar.RedrawWindow();

	ToolBarButtonInfo const buttons[] = {
		{ ID_VIEW_REFRESH, IDI_REFRESH },
		{ 0 },
		{ ID_EDIT_PROPERTIES, IDI_PROPERTIES },
		{ 0 },
		{ ID_VIEW_HIERARCHY, IDI_TREE },
		{ ID_VIEW_SESSIONS, IDI_SESSION },
		{ ID_VIEW_PROVIDERS, IDI_PROVIDER },
		{ ID_VIEW_FILTERS, IDI_FILTER },
		{ ID_VIEW_LAYERS, IDI_LAYERS },
		{ ID_VIEW_NETWORKEVENTS, IDI_EVENT },
		{ ID_VIEW_SUBLAYERS, IDI_SUBLAYER },
		{ ID_VIEW_CALLOUTS, IDI_CALLOUT },
		{ ID_VIEW_PROVIDERCONTEXTS, IDI_CONTEXT },
		{ 0 },
		{ ID_EDIT_FIND, IDI_FIND },
		{ ID_EDIT_FINDNEXT, IDI_FIND_NEXT },
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
	UINT icons[] = { 
		IDI_SESSION, IDI_FILTER, IDI_PROVIDER, IDI_LAYERS, IDI_SUBLAYER, IDI_CALLOUT,
		IDI_CONTEXT, IDI_TREE, IDI_EVENT,
	};
	for (auto icon : icons)
		images.AddIcon(AtlLoadIconImage(icon, 0, 16, 16));
	m_view.SetImageList(images);

	UISetCheck(ID_VIEW_STATUS_BAR, 1);

	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	CMenuHandle menuMain = GetMenu();
	m_view.SetWindowMenu(menuMain.GetSubMenu(WINDOW_MENU_POSITION));

	LOGFONT lf;
	((CFontHandle)m_view.GetFont()).GetLogFont(lf);
	wcscpy_s(lf.lfFaceName, L"Consolas");
	m_MonoFont.CreateFontIndirect(&lf);

	if (!m_Engine.Open()) {
		AtlMessageBox(nullptr, L"Failed to open WFP Engine", IDR_MAINFRAME, MB_ICONERROR);
	}
	InitMenu();
	UIAddMenu(menuMain);
	AddMenu(menuMain);
	UpdateUI();

	PostMessage(WM_COMMAND, ID_VIEW_HIERARCHY);

	return 0;
}

void CMainFrame::SetStatusText(int index, PCWSTR text) {
	m_StatusBar.SetText(index, text, 0);
}

CUpdateUIBase& CMainFrame::UI() {
	return *this;
}

HFONT CMainFrame::GetMonoFont() const {
	return m_MonoFont.m_hFont;
}

bool CMainFrame::TrackPopupMenu(HMENU hMenu, DWORD flags, int x, int y, HWND hWnd) {
	return ShowContextMenu(hMenu, flags, x, y, hWnd);
}

CFindReplaceDialog* CMainFrame::GetFindDialog() const {
	return m_pFindDlg;
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
		{ ID_VIEW_PROVIDERCONTEXTS, IDI_CONTEXT },
		{ ID_EDIT_PROPERTIES, IDI_PROPERTIES },
		{ ID_VIEW_HIERARCHY, IDI_TREE },
		{ ID_VIEW_NETWORKEVENTS, IDI_EVENT },
		{ ID_FILE_OPEN, IDI_OPEN },
		{ ID_FILE_SAVE, IDI_SAVE },
		{ ID_EDIT_FIND, IDI_FIND },
		{ ID_EDIT_FINDNEXT, IDI_FIND_NEXT },
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
	UIEnable(ID_EDIT_COPY, false);
	UIEnable(ID_EDIT_CUT, false);
	UIEnable(ID_EDIT_PASTE, false);
	UIEnable(ID_EDIT_DELETE, false);
	UIEnable(ID_VIEW_REFRESH, anyPage);
	UIEnable(ID_EDIT_PROPERTIES, false);
	UIEnable(ID_EDIT_FIND, anyPage);
}

void CMainFrame::SetAlwaysOnTop(bool onTop) {
	SetWindowPos(onTop ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	UISetCheck(ID_OPTIONS_ALWAYSONTOP, onTop);
}

LRESULT CMainFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	WINDOWPLACEMENT wp{ sizeof(wp) };
	GetWindowPlacement(&wp);
	AppSettings::Get().MainWindowPlacement(wp);
	auto pLoop = _Module.GetMessageLoop();
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);

	bHandled = FALSE;
	return 1;
}

LRESULT CMainFrame::OnEraseBkgnd(UINT, WPARAM, LPARAM, BOOL& bHandled) {
	return 1;
}

LRESULT CMainFrame::OnShowWindow(UINT, WPARAM, LPARAM, BOOL&) {
	static bool shown = false;
	if (!shown) {
		shown = true;
		auto wp = AppSettings::Get().MainWindowPlacement();
		if (wp.showCmd)
			SetWindowPlacement(&wp);
		SetAlwaysOnTop(AppSettings::Get().AlwaysOnTop());
	}
	return 0;
}

LRESULT CMainFrame::OnRebuildWindowMenu(UINT, WPARAM, LPARAM, BOOL& bHandled) {
	auto il = CImageList(m_view.GetImageList());
	for (int i = 0; i < m_view.GetPageCount(); i++) {
		auto image = m_view.GetPageImage(i);
		AddCommand(ID_WINDOW_TABFIRST + i, il.GetIcon(image));
	}
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

	return 0;
}

LRESULT CMainFrame::OnViewFilters(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	auto view = new CFiltersView(this, m_Engine);
	view->Create(m_view, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);
	m_view.AddPage(view->m_hWnd, L"Filters", 1, view);
	view->Refresh();

	return 0;
}

LRESULT CMainFrame::OnViewProviders(WORD, WORD, HWND, BOOL&) {
	auto view = new CProvidersView(this, m_Engine);
	view->Create(m_view, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);
	m_view.AddPage(view->m_hWnd, L"Providers", 2, view);

	return 0;
}

LRESULT CMainFrame::OnViewProviderContexts(WORD, WORD, HWND, BOOL&) {
	auto view = new CProviderContextView(this, m_Engine);
	view->Create(m_view, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);
	m_view.AddPage(view->m_hWnd, L"Provider Contexts", 6, view);

	return 0;
}

LRESULT CMainFrame::OnViewLayers(WORD, WORD, HWND, BOOL&) {
	auto view = new CLayersView(this, m_Engine);
	view->Create(m_view, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);
	m_view.AddPage(view->m_hWnd, L"Layers", 3, view);

	return 0;
}

LRESULT CMainFrame::OnViewNetEvents(WORD, WORD, HWND, BOOL&) {
	auto view = new CNetEventsView(this, m_Engine);
	view->Create(m_view, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);
	m_view.AddPage(view->m_hWnd, L"Net Events", 8, view);

	return 0;
}

LRESULT CMainFrame::OnViewHierarchy(WORD, WORD, HWND, BOOL&) {
	auto view = new CHierarchyView(this, m_Engine);
	view->Create(m_view, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);
	m_view.AddPage(view->m_hWnd, L"Hierarchy", 7, view);

	return 0;
}

LRESULT CMainFrame::OnViewSublayers(WORD, WORD, HWND, BOOL&) {
	auto view = new CSublayersView(this, m_Engine);
	view->Create(m_view, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);
	m_view.AddPage(view->m_hWnd, L"Sublayers", 4, view);

	return 0;
}

LRESULT CMainFrame::OnViewCallouts(WORD, WORD, HWND, BOOL&) {
	auto view = new CCalloutsView(this, m_Engine);
	view->Create(m_view, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);
	m_view.AddPage(view->m_hWnd, L"Callouts", 5, view);
	view->Refresh();

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

LRESULT CMainFrame::OnAlwaysOnTop(WORD, WORD, HWND, BOOL&) {
	auto& settings = AppSettings::Get();
	settings.AlwaysOnTop(!settings.AlwaysOnTop());
	SetAlwaysOnTop(settings.AlwaysOnTop());
	return 0;
}

LRESULT CMainFrame::OnEditFind(WORD, WORD, HWND, BOOL&) {
	ATLASSERT(m_view.GetPageCount() > 0);

	if (m_pFindDlg == nullptr) {
		m_pFindDlg = new CFindReplaceDialog;
		m_pFindDlg->Create(TRUE, m_SearchText, nullptr, FR_DOWN | FR_HIDEWHOLEWORD, m_hWnd);
		m_pFindDlg->ShowWindow(SW_SHOW);
	}

	return 0;
}

LRESULT CMainFrame::OnFind(UINT msg, WPARAM wp, LPARAM lp, BOOL&) {
	if (m_pFindDlg->IsTerminating()) {
		m_pFindDlg = nullptr;
		return 0;
	}
	m_pFindDlg->SetFocus();
	
	if (auto page = m_view.GetActivePage(); page >= 0) {
		m_SearchText = m_pFindDlg->GetFindString();
		::SendMessage(m_view.GetPageHWND(page), msg, wp, lp);
	}
	return 0;
}
