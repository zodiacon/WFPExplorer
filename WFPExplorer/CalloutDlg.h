#pragma once

#include <DialogHelper.h>

class WFPEngine;

class CCalloutDlg :
	public CDialogImpl<CCalloutDlg>,
	public CDialogHelper<CCalloutDlg> {
public:
	enum { IDD = IDD_CALLOUT };

	explicit CCalloutDlg(WFPEngine& engine, FWPM_CALLOUT* callout);

	BEGIN_MSG_MAP(CCalloutDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_ID_HANDLER(IDC_PROVIDER_PROP, OnShowProvider)
		COMMAND_ID_HANDLER(IDC_LAYER_PROP, OnShowLayer)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

private:
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnShowProvider(WORD /*wNotifyCode*/, WORD, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnShowLayer(WORD /*wNotifyCode*/, WORD, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	FWPM_CALLOUT* m_Callout;
	WFPEngine& m_Engine;
};
