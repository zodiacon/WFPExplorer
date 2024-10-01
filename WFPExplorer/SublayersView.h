#pragma once

#include "GenericListViewBase.h"
#include <WFPEngine.h>
#include <WFPEnumerator.h>

class WFPEngine;

class CSublayersView :
	public CGenericListViewBase<CSublayersView> {
public:
	CSublayersView(IMainFrame* frame, WFPEngine& engine);

	void Refresh();

	CString GetColumnText(HWND, int row, int col);
	void DoSort(SortInfo const* si);
	int GetSaveColumnRange(HWND, int&) const;
	int GetRowImage(HWND, int row, int col) const;
	CString GetDefaultSaveFile() const;
	bool OnDoubleClickList(HWND, int row, int col, POINT const& pt);
	bool OnRightClickList(HWND, int row, int col, POINT const& pt) const;
	void OnStateChanged(HWND, int from, int to, UINT oldState, UINT newState);

	BEGIN_MSG_MAP(CSublayersView)
		MESSAGE_HANDLER(WM_ACTIVATE, OnActivate)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		CHAIN_MSG_MAP(CGenericListViewBase<CSublayersView>)
	ALT_MSG_MAP(1)
		COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnRefresh)
		COMMAND_ID_HANDLER(ID_EDIT_PROPERTIES, OnProperties)
		COMMAND_ID_HANDLER(ID_EDIT_DELETE, OnDeleteSubLayer)
		CHAIN_MSG_MAP_ALT(CGenericListViewBase<CSublayersView>, 1)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

private:
	void UpdateUI() const;

	enum class ColumnType {
		Key, Name, Desc, Flags, Weight, Provider, ProviderData,
	};

	struct SubLayerInfo {
		FWPM_SUBLAYER* Data;
		CString ProviderName;
		CString const& Name() const;
		CString const& Desc() const;

	private:
		mutable CString m_Name, m_Desc;
	};

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnDeleteSubLayer(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnActivate(UINT, WPARAM activate, LPARAM, BOOL&);

	WFPEngine& m_Engine;

	WFPObjectVector<FWPM_SUBLAYER, SubLayerInfo> m_Layers;
};
