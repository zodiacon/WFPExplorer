#pragma once

#include "GenericListViewBase.h"
#include <WFPEngine.h>
#include "resource.h"
#include <WFPEnumerator.h>

class WFPEngine;

class CFiltersView :
	public CGenericListViewBase<CFiltersView> {
public:
	CFiltersView(IMainFrame* frame, WFPEngine& engine);

	void SetLayer(GUID const& layer);

	void Refresh();

	CString GetColumnText(HWND, int row, int col);
	void DoSort(SortInfo const* si);
	int GetSaveColumnRange(HWND, int&) const;
	int GetRowImage(HWND, int row, int col) const;
	void OnStateChanged(HWND, int from, int to, UINT oldState, UINT newState);
	bool OnDoubleClickList(HWND, int row, int col, POINT const& pt);
	bool OnRightClickList(HWND, int row, int col, POINT const& pt);
	CString GetDefaultSaveFile() const;

	BEGIN_MSG_MAP(CFiltersView)
		MESSAGE_HANDLER(WM_ACTIVATE, OnActivate)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		CHAIN_MSG_MAP(CGenericListViewBase<CFiltersView>)
	ALT_MSG_MAP(1)
		COMMAND_ID_HANDLER(ID_EDIT_PROPERTIES, OnProperties)
		COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnRefresh)
		COMMAND_ID_HANDLER(ID_EDIT_DELETE, OnDeleteFilter)
		COMMAND_ID_HANDLER(ID_EDIT_COPY, OnCopy)
		CHAIN_MSG_MAP_ALT(CGenericListViewBase<CFiltersView>, 1)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

private:
	void UpdateUI() const;

	enum class ColumnType {
		Key, Name, Desc, Flags, ProviderGUID, ProviderName, LayerKey, SubLayerKey,
		Weight, ConditionCount, Action, Id, EffectiveWeight, Layer, SubLayer, ActionKey, ProviderData,
	};

	struct FilterInfo {
		FWPM_FILTER* Data;
		CString ProviderName;
		CString Layer, SubLayer;
		CString FilterAction;
		CString const& Name() const;
		CString const& Desc() const;

	private:
		mutable CString m_Name, m_Desc;
	};

	CString const& GetProviderName(FilterInfo& info) const;
	CString const& GetLayerName(FilterInfo& info) const;
	CString const& GetSublayerName(FilterInfo& info) const;

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnActivate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDeleteFilter(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	WFPEngine& m_Engine;

	WFPObjectVector<FWPM_FILTER, FilterInfo> m_Filters;
	GUID m_Layer{ GUID_NULL };
};
