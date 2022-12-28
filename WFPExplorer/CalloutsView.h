#pragma once

#include "GenericListViewBase.h"
#include <WFPEngine.h>
#include <WFPEnumerators.h>

class WFPEngine;

class CCalloutsView :
	public CGenericListViewBase<CCalloutsView> {
public:
	CCalloutsView(IMainFrame* frame, WFPEngine& engine);

	void SetLayer(GUID const& layer);
	void Refresh();

	CString GetColumnText(HWND, int row, int col);
	void DoSort(SortInfo const* si);
	int GetSaveColumnRange(HWND, int&) const;
	int GetRowImage(HWND, int row, int col) const;
	void OnStateChanged(HWND, int from, int to, UINT oldState, UINT newState) const;
	bool OnRightClickList(HWND, int row, int col, POINT const& pt) const;
	bool OnDoubleClickList(HWND, int row, int col, POINT const& pt);
	CString GetDefaultSaveFile() const;

	BEGIN_MSG_MAP(CCalloutsView)
		MESSAGE_HANDLER(WM_ACTIVATE, OnActivate)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		CHAIN_MSG_MAP(CGenericListViewBase<CCalloutsView>)
	ALT_MSG_MAP(1)
		COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnRefresh)
		COMMAND_ID_HANDLER(ID_EDIT_DELETE, OnDelete)
		COMMAND_ID_HANDLER(ID_EDIT_PROPERTIES, OnProperties)
		COMMAND_ID_HANDLER(ID_EDIT_COPY, OnCopy)
		CHAIN_MSG_MAP_ALT(CGenericListViewBase<CCalloutsView>, 1)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

private:
	void UpdateUI() const;

	enum class ColumnType {
		Key, Name, Desc, Flags, Fields, Provider, Id, Layer, ProviderData,
	};

	struct CalloutInfo {
		FWPM_CALLOUT* Data;
		mutable CString Provider;
		mutable CString Layer;
		mutable CString Name, Desc;
	};

	CString const& GetCalloutProvider(CalloutInfo& info) const;
	CString const& GetCalloutName(CalloutInfo& info) const;
	CString const& GetCalloutDesc(CalloutInfo& info) const;
	CString const& GetCalloutLayer(CalloutInfo& info) const;

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnActivate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) const;
	LRESULT OnRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnDelete(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	WFPEngine& m_Engine;

	WFPObjectVector<FWPM_CALLOUT, CalloutInfo> m_Callouts;
	GUID m_LayerKey{ GUID_NULL };
};
