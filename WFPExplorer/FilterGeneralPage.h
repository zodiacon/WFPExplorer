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

	CFilterGeneralPage(WFPEngine& engine, FWPM_FILTER* filter);

	BEGIN_MSG_MAP(CFilterGeneralPage)
		COMMAND_ID_HANDLER(IDC_LAYER_PROP, OnShowLayer)
		COMMAND_ID_HANDLER(IDC_PROVIDER_PROP, OnShowProvider)
		COMMAND_ID_HANDLER(IDC_CALLOUT_PROP, OnShowCallout)
		COMMAND_ID_HANDLER(IDC_SUBLAYER_PROP, OnShowSublayer)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

private:
	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnShowLayer(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnShowProvider(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnShowSublayer(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnShowCallout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
	WFPEngine& m_Engine;
	FWPM_FILTER* m_Filter;
};
