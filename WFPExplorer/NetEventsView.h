#pragma once

#include "GenericListViewBase.h"
#include <WFPEngine.h>
#include "resource.h"
#include <WFPEnumerator.h>
#include "PropertiesListDlg.h"

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
	bool OnDoubleClickList(HWND, int row, int col, POINT const& pt);
	CString GetDefaultSaveFile() const;

	BEGIN_MSG_MAP(CNetEventsView)
		MESSAGE_HANDLER(WM_ACTIVATE, OnActivate)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		CHAIN_MSG_MAP(CGenericListViewBase<CNetEventsView>)
	ALT_MSG_MAP(1)
		COMMAND_ID_HANDLER(ID_EDIT_PROPERTIES, OnProperties)
		COMMAND_ID_HANDLER(ID_EDIT_COPY, OnCopy)
		COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnRefresh)
		CHAIN_MSG_MAP_ALT(CGenericListViewBase<CNetEventsView>, 1)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

private:
	// the properties of the event's type (empty when the type doesn't have them)
	struct EventDetails {
		bool HasFilter{ false }, HasLayer{ false }, HasClassify{ false };
		UINT64 FilterId{ 0 };
		UINT16 LayerId{ 0 };
		UINT32 ReauthReason{ 0 }, OriginalProfile{ 0 }, CurrentProfile{ 0 };
		bool Loopback{ false };
		CString Direction, Error, Capability, Spi, LocalMac, RemoteMac;
	};

	struct NetEventInfo {
		FWPM_NET_EVENT* Data;
		CString LocalAddress, RemoteAddress;
		CString AppId, UserId, PackageId;
		EventDetails Details;
	};

	void UpdateUI() const;
	CString const& GetLocalAddress(NetEventInfo& info) const;
	CString const& GetRemoteAddress(NetEventInfo& info) const;
	UINT16 GetLocalPort(NetEventInfo& info) const;
	UINT16 GetRemotePort(NetEventInfo& info) const;
	CString const& GetAppId(NetEventInfo& info);
	CString const& GetUserId(NetEventInfo& info);
	CString const& GetPackageId(NetEventInfo& info);
	CString const& GetFilterName(UINT64 id);
	CString const& GetLayerName(UINT16 id);
	static EventDetails GetDetails(FWPM_NET_EVENT const* e);

	enum class ColumnType {
		Time, Type, LocalPort, RemotePort, LocalAddress, RemoteAddress, Flags, EnterpriseId,
		IPVersion, Protocol, ScopeId, AppId, UserId, PackageId, PolicyFlags, EffectiveName,
		AddressFamily, FilterId, Filter, Layer, Direction, Loopback, ReauthReason, OriginalProfile, CurrentProfile,
		Error, Capability, Spi, LocalMac, RemoteMac,
	};

	CString GetText(NetEventInfo& info, ColumnType column);
	// the columns' values and what the columns don't show, for the properties dialog
	std::vector<std::pair<CString, CString>> GetProperties(NetEventInfo& info);

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnActivate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	WFPEngine& m_Engine;

	WFPObjectVector<FWPM_NET_EVENT, NetEventInfo> m_Events;
	// events share a few filters and layers
	std::unordered_map<UINT64, CString> m_FilterNames;
	std::unordered_map<UINT16, CString> m_LayerNames;
};
