#pragma once

#include <DialogHelper.h>
#include "resource.h"

class WFPEngine;
struct WFPFilterInfo;

class CFilterGeneralPage :
	public CPropertyPageImpl<CFilterGeneralPage>,
	public CDialogHelper<CFilterGeneralPage> {
public:
	enum { IDD = IDD_FILTERINFO };

	CFilterGeneralPage(WFPEngine& engine, WFPFilterInfo& filter);

	BEGIN_MSG_MAP(CFilterGeneralPage)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

private:
	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

private:
	WFPEngine& m_Engine;
	WFPFilterInfo& m_Filter;
};
