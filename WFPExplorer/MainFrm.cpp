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
#include "ProviderDlg.h"
#include "NewFilterDlg.h"
#include "NewSubLayerDlg.h"

const int WINDOW_MENU_POSITION = 4;

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg) {
	if (m_pFindDlg && m_pFindDlg->IsDialogMessageW(pMsg))
		return TRUE;

	if (CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
		return TRUE;

	return m_Tabs.PreTranslateMessage(pMsg);
}

BOOL CMainFrame::OnIdle() {
	UIUpdateToolBar();
	UIUpdateStatusBar();
	return FALSE;
}

LRESULT CMainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	if (!m_Engine.Open()) {
		AtlMessageBox(nullptr, L"Failed to open WFP Engine", IDR_MAINFRAME, MB_ICONERROR);
		return -1;
	}

	SetCheckIcon(IDI_CHECK, IDI_RADIO);
	InitDarkTheme();
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

	//m_view.m_bTabCloseButton = false;
	m_hWndClient = m_Tabs.Create(m_hWnd, rcDefault, nullptr,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);
	CImageList images;
	images.Create(16, 16, ILC_COLOR32, 8, 4);
	UINT icons[] = {
		IDI_SESSION, IDI_FILTER, IDI_PROVIDER, IDI_LAYERS, IDI_SUBLAYER, IDI_CALLOUT,
		IDI_CONTEXT, IDI_TREE, IDI_EVENT,
	};
	for (auto icon : icons)
		images.AddIcon(AtlLoadIconImage(icon, 0, 16, 16));
	m_Tabs.SetImageList(images);

	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	CMenuHandle menuMain = GetMenu();
	m_Tabs.SetWindowMenu(menuMain.GetSubMenu(WINDOW_MENU_POSITION));

	LOGFONT lf;
	((CFontHandle)m_Tabs.GetFont()).GetLogFont(lf);
	wcscpy_s(lf.lfFaceName, L"Consolas");
	m_MonoFont.CreateFontIndirect(&lf);

	if (!m_Engine.Open()) {
		AtlMessageBox(nullptr, L"Failed to open WFP Engine", IDR_MAINFRAME, MB_ICONERROR);
	}

	InitMenu();
	UIAddMenu(menuMain);
	AddMenu(menuMain);
	UISetCheck(ID_VIEW_STATUS_BAR, 1);

	UpdateUI();

	if (AppSettings::Get().DarkMode()) {
		ThemeHelper::SetCurrentTheme(s_DarkTheme, m_hWnd);
		ThemeHelper::UpdateMenuColors(*this, true);
		UpdateMenu(GetMenu(), true);
		UISetCheck(ID_OPTIONS_DARKMODE, true);
	}

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
		{ ID_NEW_FILTER, IDI_FILTER },
		{ ID_NEW_PROVIDER, IDI_PROVIDER },
		{ ID_NEW_SUBLAYER, IDI_SUBLAYER },
	};
	for (auto& cmd : cmds) {
		if (cmd.icon)
			AddCommand(cmd.id, cmd.icon);
		else
			AddCommand(cmd.id, cmd.hIcon);
	}
}

void CMainFrame::UpdateUI() {
	bool anyPage = m_Tabs.GetPageCount() > 0;
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
	auto il = CImageList(m_Tabs.GetImageList());
	for (int i = 0; i < m_Tabs.GetPageCount(); i++) {
		auto image = m_Tabs.GetPageImage(i);
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
	view->Create(m_Tabs, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);
	m_Tabs.AddPage(view->m_hWnd, L"Sessions", 0, view);

	return 0;
}

LRESULT CMainFrame::OnViewFilters(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	auto view = new CFiltersView(this, m_Engine);
	view->Create(m_Tabs, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);
	m_Tabs.AddPage(view->m_hWnd, L"Filters", 1, view);
	view->Refresh();

	return 0;
}

LRESULT CMainFrame::OnViewProviders(WORD, WORD, HWND, BOOL&) {
	auto view = new CProvidersView(this, m_Engine);
	view->Create(m_Tabs, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);
	m_Tabs.AddPage(view->m_hWnd, L"Providers", 2, view);

	return 0;
}

LRESULT CMainFrame::OnViewProviderContexts(WORD, WORD, HWND, BOOL&) {
	auto view = new CProviderContextView(this, m_Engine);
	view->Create(m_Tabs, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);
	m_Tabs.AddPage(view->m_hWnd, L"Provider Contexts", 6, view);

	return 0;
}

LRESULT CMainFrame::OnViewLayers(WORD, WORD, HWND, BOOL&) {
	auto view = new CLayersView(this, m_Engine);
	view->Create(m_Tabs, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);
	m_Tabs.AddPage(view->m_hWnd, L"Layers", 3, view);

	return 0;
}

LRESULT CMainFrame::OnViewNetEvents(WORD, WORD, HWND, BOOL&) {
	auto view = new CNetEventsView(this, m_Engine);
	view->Create(m_Tabs, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);
	m_Tabs.AddPage(view->m_hWnd, L"Net Events", 8, view);

	return 0;
}

LRESULT CMainFrame::OnViewHierarchy(WORD, WORD, HWND, BOOL&) {
	auto view = new CHierarchyView(this, m_Engine);
	view->Create(m_Tabs, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);
	m_Tabs.AddPage(view->m_hWnd, L"Hierarchy", 7, view);

	return 0;
}

LRESULT CMainFrame::OnViewSublayers(WORD, WORD, HWND, BOOL&) {
	auto view = new CSublayersView(this, m_Engine);
	view->Create(m_Tabs, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);
	m_Tabs.AddPage(view->m_hWnd, L"Sublayers", 4, view);

	return 0;
}

LRESULT CMainFrame::OnViewCallouts(WORD, WORD, HWND, BOOL&) {
	auto view = new CCalloutsView(this, m_Engine);
	view->Create(m_Tabs, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);
	m_Tabs.AddPage(view->m_hWnd, L"Callouts", 5, view);
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
	int nActivePage = m_Tabs.GetActivePage();
	if (nActivePage != -1)
		m_Tabs.RemovePage(nActivePage);
	else
		::MessageBeep((UINT)-1);

	return 0;
}

LRESULT CMainFrame::OnWindowCloseAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	m_Tabs.RemoveAllPages();

	return 0;
}

LRESULT CMainFrame::OnWindowActivate(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int nPage = wID - ID_WINDOW_TABFIRST;
	m_Tabs.SetActivePage(nPage);

	return 0;
}

LRESULT CMainFrame::OnPageActivated(int, LPNMHDR, BOOL&) {
	int page = m_Tabs.GetActivePage();
	bool handled = false;
	if (page >= 0) {
		handled = ::SendMessage(m_Tabs.GetPageHWND(page), WM_ACTIVATE, 1, 0);
	}
	if (!handled) {
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
	ATLASSERT(m_Tabs.GetPageCount() > 0);

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

	if (auto page = m_Tabs.GetActivePage(); page >= 0) {
		m_SearchText = m_pFindDlg->GetFindString();
		::SendMessage(m_Tabs.GetPageHWND(page), msg, wp, lp);
	}
	return 0;
}

LRESULT CMainFrame::OnToggleDarkMode(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	auto& settings = AppSettings::Get();
	settings.DarkMode(!settings.DarkMode());
	UISetCheck(ID_OPTIONS_DARKMODE, settings.DarkMode());

	if (settings.DarkMode())
		ThemeHelper::SetCurrentTheme(s_DarkTheme, m_hWnd);
	else
		ThemeHelper::SetDefaultTheme(m_hWnd);

	ThemeHelper::UpdateMenuColors(*this, settings.DarkMode());
	UpdateMenu(GetMenu(), true);

	return 0;
}

LRESULT CMainFrame::OnNewProvider(WORD, WORD, HWND, BOOL&) {
	FWPM_PROVIDER provider{};
	CProviderDlg dlg(&provider);
	if (dlg.DoModal() == IDOK) {
		//
		// attempt to add a new provider
		//
		WFPEngine engine;
		bool ok = false;
		if (engine.Open()) {
			ok = engine.AddProvider(&provider);
		}
		AtlMessageBox(m_hWnd, ok ? L"Provider added successfully." : L"Failed to add provider.",
			IDS_TITLE, ok ? MB_ICONINFORMATION : MB_ICONERROR);
	}

	return 0;
}

LRESULT CMainFrame::OnNewFilter(WORD, WORD, HWND, BOOL&) {
	FWPM_FILTER filter{};
	CNewFilterDlg dlg(&filter);
	if (dlg.DoModal() == IDOK) {
		WFPEngine engine;
		auto ok = engine.Open();
		UINT64 id = 0;
		if (ok) {
			id = engine.AddFilter(&filter);
		}
		AtlMessageBox(m_hWnd, ok && id ? std::format(L"Filter with ID {} added successfully.", id).c_str() : L"Failed to add filter.",
			IDS_TITLE, ok ? MB_ICONINFORMATION : MB_ICONERROR);

	}
	return 0;
}

LRESULT CMainFrame::OnNewSubLayer(WORD, WORD, HWND, BOOL&) {
	FWPM_SUBLAYER sl{};
	CNewSubLayerDlg dlg(&sl);
	if (IDOK == dlg.DoModal()) {
		WFPEngine engine;
		auto ok = engine.Open();
		if (ok) {
			ok = engine.AddSubLayer(&sl);
		}
		AtlMessageBox(m_hWnd, ok ? L"Sublayer added successfully." : L"Failed to add sublayer.",
			IDS_TITLE, ok ? MB_ICONINFORMATION : MB_ICONERROR);
	}
	return 0;
}

void CMainFrame::InitDarkTheme() const {
	s_DarkTheme.BackColor = s_DarkTheme.SysColors[COLOR_WINDOW] = RGB(32, 32, 32);
	s_DarkTheme.TextColor = s_DarkTheme.SysColors[COLOR_WINDOWTEXT] = RGB(248, 248, 248);
	s_DarkTheme.SysColors[COLOR_HIGHLIGHT] = RGB(10, 10, 160);
	s_DarkTheme.SysColors[COLOR_HIGHLIGHTTEXT] = RGB(240, 240, 240);
	s_DarkTheme.SysColors[COLOR_MENUTEXT] = s_DarkTheme.TextColor;
	s_DarkTheme.SysColors[COLOR_CAPTIONTEXT] = s_DarkTheme.TextColor;
	s_DarkTheme.SysColors[COLOR_BTNFACE] = s_DarkTheme.BackColor;
	s_DarkTheme.SysColors[COLOR_BTNTEXT] = s_DarkTheme.TextColor;
	s_DarkTheme.SysColors[COLOR_3DLIGHT] = RGB(192, 192, 192);
	s_DarkTheme.SysColors[COLOR_BTNHIGHLIGHT] = RGB(192, 192, 192);
	s_DarkTheme.SysColors[COLOR_CAPTIONTEXT] = s_DarkTheme.TextColor;
	s_DarkTheme.SysColors[COLOR_3DSHADOW] = s_DarkTheme.TextColor;
	s_DarkTheme.SysColors[COLOR_SCROLLBAR] = s_DarkTheme.BackColor;
	s_DarkTheme.SysColors[COLOR_APPWORKSPACE] = s_DarkTheme.BackColor;
	s_DarkTheme.StatusBar.BackColor = RGB(16, 0, 16);
	s_DarkTheme.StatusBar.TextColor = s_DarkTheme.TextColor;

	s_DarkTheme.Name = L"Dark";
	s_DarkTheme.Menu.BackColor = s_DarkTheme.BackColor;
	s_DarkTheme.Menu.TextColor = s_DarkTheme.TextColor;
}
