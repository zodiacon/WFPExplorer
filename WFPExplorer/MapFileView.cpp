#include "pch.h"
#include "MapFileView.h"
#include "WFPHelper.h"

using namespace NodeGraphCtrl;

CMapFileView::CMapFileView(IMainFrame* frame) : CFrameView(frame) {
}

bool CMapFileView::Load(PCWSTR path) {
	auto model = std::make_unique<NodeGraphModel>();
	if (!model->Load(path))
		return false;
	m_Graph.SetModel(model.release());
	m_Path = path;

	// fitting needs the graph's size, which a new view doesn't have yet
	CRect rc;
	m_Graph.GetClientRect(&rc);
	if (rc.IsRectEmpty())
		m_NeedFit = true;
	else
		WFPHelper::FitGraph(m_Graph);
	return true;
}

CString CMapFileView::GetTitle() const {
	return CString(L"Map File: ") + ::PathFindFileName(m_Path);
}

void CMapFileView::UpdateBackground() {
	m_Graph.SetBackgroundColor(WTLHelper::IsDarkMode() ? RGB(38, 38, 38) : RGB(245, 245, 245));
}

LRESULT CMapFileView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	m_hWndClient = m_Graph.Create(m_hWnd, 0, 0, 0, 0, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, NGCS_READONLY);
	m_Graph.SetMinimapVisible(true);
	UpdateBackground();
	return 0;
}

LRESULT CMapFileView::OnSize(UINT, WPARAM wp, LPARAM, BOOL&) {
	if (wp != SIZE_MINIMIZED) {
		UpdateLayout();
		CRect rc;
		m_Graph.GetClientRect(&rc);
		if (m_NeedFit && !rc.IsRectEmpty()) {
			m_NeedFit = false;
			WFPHelper::FitGraph(m_Graph);
		}
	}
	return 0;
}

LRESULT CMapFileView::OnThemeChanged(UINT, WPARAM, LPARAM, BOOL& handled) {
	UpdateBackground();
	handled = FALSE;
	return 0;
}

//
// the file may have been saved again since
//
LRESULT CMapFileView::OnRefresh(WORD, WORD, HWND, BOOL&) {
	if (!Load(m_Path))
		AtlMessageBox(m_hWnd, L"Failed to read the map file", IDS_TITLE, MB_ICONERROR);
	return 0;
}

LRESULT CMapFileView::OnSave(WORD, WORD, HWND, BOOL&) {
	CString name(::PathFindFileName(m_Path));
	::PathRemoveExtension(name.GetBuffer());
	name.ReleaseBuffer();
	WFPHelper::SaveMap(m_hWnd, m_Graph, name);
	return 0;
}
