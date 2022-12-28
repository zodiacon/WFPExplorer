#pragma once

#include "GenericListViewBase.h"
#include <WFPEngine.h>
#include <WFPEnumerators.h>

class WFPEngine;

class CSessionsView : 
	public CGenericListViewBase<CSessionsView> {
public:
	CSessionsView(IMainFrame* frame, WFPEngine& engine);

	BOOL PreTranslateMessage(MSG* pMsg);
	void Refresh();

	CString GetColumnText(HWND, int row, int col);
	void DoSort(SortInfo const* si);
	int GetRowImage(HWND, int row, int col) const;
	CString GetDefaultSaveFile() const;

	BEGIN_MSG_MAP(CSessionsView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		CHAIN_MSG_MAP(CGenericListViewBase<CSessionsView>)
	ALT_MSG_MAP(1)
		COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnRefresh)
		CHAIN_MSG_MAP_ALT(CGenericListViewBase<CSessionsView>, 1)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

private:
	enum class ColumnType {
		Key, Name, Desc, Flags, ProcessId, ProcessName, UserName, SID, KernelMode,
	};
	struct SessionInfo {
		FWPM_SESSION* Data;
		CString ProcessName;
	};

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	WFPEngine& m_Engine;

	WFPObjectVector<FWPM_SESSION, SessionInfo> m_Sessions;
};
