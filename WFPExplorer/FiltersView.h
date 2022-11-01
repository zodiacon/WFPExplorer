#pragma once

#include <FrameView.h>
#include <VirtualListView.h>
#include "Interfaces.h"
#include <WFPEngine.h>

class WFPEngine;

class CFiltersView :
	public CFrameView<CFiltersView, IMainFrame>,
	public CVirtualListView<CFiltersView> {
public:
	CFiltersView(IMainFrame* frame, WFPEngine& engine);

	void Refresh();

	CString GetColumnText(HWND, int row, int col);

	BEGIN_MSG_MAP(CFiltersView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		CHAIN_MSG_MAP(CVirtualListView<CFiltersView>)
		CHAIN_MSG_MAP(BaseFrame)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

private:
	enum class ColumnType {
		Key, Name, Desc, Flags, ProviderGUID, ProviderName, LayerKey, SubLayerKey,
		Weight, ConditionCount, Action, Id, EffectiveWeight,
	};

	struct FilterInfo : WFPFilterInfo {
		CString ProviderName;
	};

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	WFPEngine& m_Engine;

	CListViewCtrl m_List;
	std::vector<FilterInfo> m_Filters;
};
