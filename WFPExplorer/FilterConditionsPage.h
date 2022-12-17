#pragma once

#include <DialogHelper.h>
#include "resource.h"
#include <VirtualListView.h>

class WFPEngine;
struct WFPFilterInfo;
struct WFPFilterCondition;

class CFilterConditionsPage :
	public CPropertyPageImpl<CFilterConditionsPage>,
	public CDynamicDialogLayout<CFilterConditionsPage>,
	public CVirtualListView<CFilterConditionsPage>,
	public CDialogHelper<CFilterConditionsPage> {
public:
	enum { IDD = IDD_FILTERCONDITIONS };

	CFilterConditionsPage(WFPEngine& engine, FWPM_FILTER* filter);

	CString GetColumnText(HWND, int row, int col) const;
	void OnStateChanged(HWND, int from, int to, UINT oldState, UINT newState);
	int GetRowImage(HWND, int row, int) const;

	BEGIN_MSG_MAP(CFilterConditionsPage)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(CVirtualListView<CFilterConditionsPage>)
		CHAIN_MSG_MAP(CDynamicDialogLayout<CFilterConditionsPage>)
	END_MSG_MAP()

private:
	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

private:
	WFPEngine& m_Engine;
	FWPM_FILTER* m_Filter;
	std::vector<FWPM_FILTER_CONDITION> m_Conditions;
	CListViewCtrl m_List;
};
