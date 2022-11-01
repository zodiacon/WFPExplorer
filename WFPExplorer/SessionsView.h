#pragma once

#include <FrameView.h>
#include <VirtualListView.h>
#include "Interfaces.h"
#include <WFPEngine.h>

class WFPEngine;

class CSessionsView : 
	public CFrameView<CSessionsView, IMainFrame>,
	public CVirtualListView<CSessionsView> {
public:
	CSessionsView(IMainFrame* frame, WFPEngine& engine);

	BOOL PreTranslateMessage(MSG* pMsg);
	void Refresh();

	CString GetColumnText(HWND, int row, int col);

	BEGIN_MSG_MAP(CSessionsView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		CHAIN_MSG_MAP(CVirtualListView<CSessionsView>)
		CHAIN_MSG_MAP(BaseFrame)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

private:
	enum class ColumnType {
		Key, Name, Desc, Flags, ProcessId, ProcessName,
	};
	struct SessionInfo : WFPSessionInfo {
		CString ProcessName;
	};

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	WFPEngine& m_Engine;

	CListViewCtrl m_List;
	std::vector<SessionInfo> m_Sessions;
};
