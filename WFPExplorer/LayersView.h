#pragma once

#include "GenericListViewBase.h"
#include <WFPEngine.h>
#include "resource.h"
#include <WFPEnumerator.h>

class WFPEngine;

class CLayersView :
	public CGenericListViewBase<CLayersView> {
public:
	CLayersView(IMainFrame* frame, WFPEngine& engine);

	void Refresh();

	CString GetColumnText(HWND, int row, int col);
	void DoSort(SortInfo const* si);
	int GetSaveColumnRange(HWND, int&) const;
	int GetRowImage(HWND, int row, int col) const;
	void OnStateChanged(HWND, int from, int to, UINT oldState, UINT newState);
	bool OnDoubleClickList(HWND, int row, int col, POINT const& pt);
	bool OnRightClickList(HWND, int row, int col, POINT const& pt);
	CString GetDefaultSaveFile() const;

	BEGIN_MSG_MAP(CLayersView)
		MESSAGE_HANDLER(WM_ACTIVATE, OnActivate)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		CHAIN_MSG_MAP(CGenericListViewBase<CLayersView>)
	ALT_MSG_MAP(1)
		COMMAND_ID_HANDLER(ID_EDIT_PROPERTIES, OnProperties)
		COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnRefresh)
		COMMAND_ID_HANDLER(ID_EDIT_COPY, OnCopy)
		CHAIN_MSG_MAP_ALT(CGenericListViewBase<CLayersView>, 1)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

private:
	struct LayerInfo {
		FWPM_LAYER* Data;
		CString DefaultSublayer;
		int FilterCount;
		int CalloutCount;
		CString const& Name() const;
		CString const& Desc() const;

	private:
		mutable CString m_Name, m_Desc;
	};

	void UpdateUI();
	int GetFilterCount(LayerInfo& layer) const;
	int GetCalloutCount(LayerInfo& layer) const;

	enum class ColumnType {
		Key, Name, Desc, Flags, Fields, DefaultSubLayer, Id, Filters, Callouts,
	};

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnActivate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCopy(WORD, WORD, HWND, BOOL&);

	WFPEngine& m_Engine;

	WFPObjectVector<FWPM_LAYER, LayerInfo> m_Layers;
};
