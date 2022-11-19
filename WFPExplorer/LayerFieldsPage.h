#pragma once

#include <DialogHelper.h>
#include "resource.h"
#include <VirtualListView.h>

class WFPEngine;
struct WFPLayerInfo;
struct WFPFieldInfo;

class CLayerFieldsPage :
	public CPropertyPageImpl<CLayerFieldsPage>,
	public CDynamicDialogLayout<CLayerFieldsPage>,
	public CVirtualListView<CLayerFieldsPage>,
	public CDialogHelper<CLayerFieldsPage> {
public:
	enum { IDD = IDD_LAYERFIELDS };

	CLayerFieldsPage(WFPEngine& engine, WFPLayerInfo& layer);

	CString GetColumnText(HWND, int row, int col) const;
	void DoSort(SortInfo const* si);

	BEGIN_MSG_MAP(CLayerFieldsPage)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(CVirtualListView<CLayerFieldsPage>)
		CHAIN_MSG_MAP(CDynamicDialogLayout<CLayerFieldsPage>)
	END_MSG_MAP()

private:
	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

private:
	WFPEngine& m_Engine;
	WFPLayerInfo& m_Layer;
	std::vector<WFPFieldInfo> m_Fields;
	CListViewCtrl m_List;
};
