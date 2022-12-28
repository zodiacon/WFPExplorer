#pragma once

#include "GenericListViewBase.h"
#include <WFPEngine.h>
#include "resource.h"
#include <WFPEnumerator.h>

class WFPEngine;

class CNetEventsView :
	public CGenericListViewBase<CNetEventsView> {
public:
	CNetEventsView(IMainFrame* frame, WFPEngine& engine);

	void Refresh();

	CString GetColumnText(HWND, int row, int col);
	void DoSort(SortInfo const* si);
	//int GetSaveColumnRange(HWND, int&) const;
	int GetRowImage(HWND, int row, int col) const;
	void OnStateChanged(HWND, int from, int to, UINT oldState, UINT newState);
	//bool OnDoubleClickList(HWND, int row, int col, POINT const& pt);
	CString GetDefaultSaveFile() const;

	BEGIN_MSG_MAP(CNetEventsView)
		MESSAGE_HANDLER(WM_ACTIVATE, OnActivate)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		CHAIN_MSG_MAP(CGenericListViewBase<CNetEventsView>)
	ALT_MSG_MAP(1)
		//COMMAND_ID_HANDLER(ID_EDIT_PROPERTIES, OnProperties)
		COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnRefresh)
		CHAIN_MSG_MAP_ALT(CGenericListViewBase<CNetEventsView>, 1)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

private:
	struct NetEventInfo {
		FWPM_NET_EVENT* Data;
		CString LocalAddress, RemoteAddress;
		CString AppId, UserId, PackageId;
	};

	void UpdateUI() const;
	CString const& GetLocalAddress(NetEventInfo& info) const;
	CString const& GetRemoteAddress(NetEventInfo& info) const;
	UINT16 GetLocalPort(NetEventInfo& info) const;
	UINT16 GetRemotePort(NetEventInfo& info) const;
	CString const& GetAppId(NetEventInfo& info);
	CString const& GetUserId(NetEventInfo& info);
	CString const& GetPackageId(NetEventInfo& info);

	enum class ColumnType {
		Time, Type, LocalPort, RemotePort, LocalAddress, RemoteAddress, Flags, EnterpriseId,
		IPVersion, Protocol, ScopeId, AppId, UserId, PackageId, PolicyFlags, EffectiveName,
		AddressFamily
		
	};

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnActivate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	WFPEngine& m_Engine;

	WFPObjectVector<FWPM_NET_EVENT, NetEventInfo> m_Events;
};
