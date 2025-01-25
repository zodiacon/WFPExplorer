#pragma once

#include <DialogHelper.h>
#include "resource.h"
#include <VirtualListView.h>
#include <WFPEnumerators.h>

class WFPEngine;

class CFiltersListPage :
	public CPropertyPageImpl<CFiltersListPage>,
	public CVirtualListView<CFiltersListPage>,
	public CDynamicDialogLayout<CFiltersListPage>,
	public CDialogHelper<CFiltersListPage> {
public:
	enum { IDD = IDD_LAYERFIELDS };

	CFiltersListPage(WFPEngine& engine, FWPM_LAYER* layer);

	size_t GetFilterCount() const;
	CString GetColumnText(HWND, int row, int col) const;
	void DoSort(SortInfo const* si);
	int GetRowImage(HWND, int row, int) const;

	BEGIN_MSG_MAP(CFiltersListPage)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(CDynamicDialogLayout<CFiltersListPage>)
		CHAIN_MSG_MAP(CVirtualListView<CFiltersListPage>)
	END_MSG_MAP()

private:
	struct FilterInfo {
		FWPM_FILTER* Data;
		int Index;
	};

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

private:
	WFPEngine& m_Engine;
	FWPM_LAYER* m_Layer;
	WFPObjectVector<FWPM_FILTER, FilterInfo> m_Filters;
	CListViewCtrl m_List;
};
