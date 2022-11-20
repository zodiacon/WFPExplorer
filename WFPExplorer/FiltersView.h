#pragma once

#include <FrameView.h>
#include <VirtualListView.h>
#include "Interfaces.h"
#include <WFPEngine.h>
#include "resource.h"

class WFPEngine;

class CFiltersView :
	public CFrameView<CFiltersView, IMainFrame>,
	public CCustomDraw<CFiltersView>,
	public CVirtualListView<CFiltersView> {
public:
	CFiltersView(IMainFrame* frame, WFPEngine& engine);

	void Refresh();

	CString GetColumnText(HWND, int row, int col);
	void DoSort(SortInfo const* si);
	int GetSaveColumnRange(HWND, int&) const;
	int GetRowImage(HWND, int row, int col) const;
	void OnStateChanged(HWND, int from, int to, UINT oldState, UINT newState);
	bool OnDoubleClickList(HWND, int row, int col, POINT const& pt);

	DWORD OnPrePaint(int, LPNMCUSTOMDRAW cd);
	DWORD OnItemPrePaint(int, LPNMCUSTOMDRAW cd);
	DWORD OnSubItemPrePaint(int, LPNMCUSTOMDRAW cd);

	BEGIN_MSG_MAP(CFiltersView)
		MESSAGE_HANDLER(WM_ACTIVATE, OnActivate)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		CHAIN_MSG_MAP(CVirtualListView<CFiltersView>)
		CHAIN_MSG_MAP(CCustomDraw<CFiltersView>)
		CHAIN_MSG_MAP(BaseFrame)
	ALT_MSG_MAP(1)
		COMMAND_ID_HANDLER(ID_EDIT_PROPERTIES, OnProperties)
		COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnRefresh)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

private:
	void UpdateUI();

	enum class ColumnType {
		Key, Name, Desc, Flags, ProviderGUID, ProviderName, LayerKey, SubLayerKey,
		Weight, ConditionCount, Action, Id, EffectiveWeight, Layer, SubLayer, ActionKey, ProviderData,
	};

	struct FilterInfo : WFPFilterInfo {
		CString ProviderName;
		CString Layer, SubLayer;
		CString FilterAction;
	};

	CString const& GetProviderName(FilterInfo& info) const;
	CString const& GetLayerName(FilterInfo& info) const;
	CString const& GetSublayerName(FilterInfo& info) const;

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnActivate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	WFPEngine& m_Engine;

	CListViewCtrl m_List;
	std::vector<FilterInfo> m_Filters;
};
