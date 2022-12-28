#pragma once

#include "GenericListViewBase.h"
#include <WFPEngine.h>
#include <WFPEnumerators.h>
#include "resource.h"

class WFPEngine;

class CProvidersView :
	public CGenericListViewBase<CProvidersView> {
public:
	CProvidersView(IMainFrame* frame, WFPEngine& engine);

	void Refresh();

	CString GetColumnText(HWND, int row, int col);
	void DoSort(SortInfo const* si);
	int GetSaveColumnRange(HWND, int&) const;
	int GetRowImage(HWND, int row, int col) const;
	void OnStateChanged(HWND, int from, int to, UINT oldState, UINT newState) const;
	bool OnDoubleClickList(HWND, int row, int col, POINT const& pt);
	bool OnRightClickList(HWND, int row, int col, POINT const& pt) const;
	CString GetDefaultSaveFile() const;

	BEGIN_MSG_MAP(CProvidersView)
		MESSAGE_HANDLER(WM_ACTIVATE, OnActivate)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		CHAIN_MSG_MAP(CGenericListViewBase<CProvidersView>)
	ALT_MSG_MAP(1)
		COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnRefresh)
		COMMAND_ID_HANDLER(ID_EDIT_PROPERTIES, OnProperties)
		COMMAND_ID_HANDLER(ID_EDIT_COPY, OnCopy)
		COMMAND_ID_HANDLER(ID_EDIT_DELETE, OnDelete)
		CHAIN_MSG_MAP_ALT(CGenericListViewBase<CProvidersView>, 1)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

private:
	void UpdateUI() const;

	enum class ColumnType {
		Key, Name, Desc, Flags, ServiceName, ProviderData,
	};
	struct ProviderInfo {
		FWPM_PROVIDER* Data;
		CString const& Name() const;
		CString const& Desc() const;

	private:
		mutable CString m_Name, m_Desc;
	};

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnActivate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) const;
	LRESULT OnRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnDelete(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	WFPEngine& m_Engine;

	WFPObjectVector<FWPM_PROVIDER, ProviderInfo> m_Providers;
};
