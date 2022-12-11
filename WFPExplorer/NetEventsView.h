#pragma once

#include <FrameView.h>
#include <VirtualListView.h>
#include "Interfaces.h"
#include <WFPEngine.h>
#include "resource.h"
#include <WFPEnumerator.h>

class WFPEngine;

class CNetEventsView :
	public CFrameView<CNetEventsView, IMainFrame>,
	public CVirtualListView<CNetEventsView> {
public:
	CNetEventsView(IMainFrame* frame, WFPEngine& engine);

	void Refresh();

	CString GetColumnText(HWND, int row, int col);
	void DoSort(SortInfo const* si);
	//int GetSaveColumnRange(HWND, int&) const;
	int GetRowImage(HWND, int row, int col) const;
	//void OnStateChanged(HWND, int from, int to, UINT oldState, UINT newState);
	//bool OnDoubleClickList(HWND, int row, int col, POINT const& pt);

	BEGIN_MSG_MAP(CNetEventsView)
		MESSAGE_HANDLER(WM_ACTIVATE, OnActivate)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		CHAIN_MSG_MAP(CVirtualListView<CNetEventsView>)
		CHAIN_MSG_MAP(BaseFrame)
	ALT_MSG_MAP(1)
		//COMMAND_ID_HANDLER(ID_EDIT_PROPERTIES, OnProperties)
		COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnRefresh)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

private:
	void UpdateUI();

	enum class ColumnType {
		Time, Type, LocalPort, RemotePort, LocalAddress, RemoteAddress, Flags, EnterpriseId,
		IPVersion, Protocol, ScopeId, AppId, UserId, PackageId, PolicyFlags, EffectiveName,
		AddressFamily
		
	};

	struct NetEventInfo {
		FWPM_NET_EVENT* Data;
	};

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnActivate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	WFPEngine& m_Engine;

	CListViewCtrl m_List;
	WFPObjectVector<FWPM_NET_EVENT, NetEventInfo> m_Events;
};
