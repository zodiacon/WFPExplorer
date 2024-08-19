#pragma once

#include <DialogHelper.h>

class WFPEngine;

class CSubLayerDlg :
	public CDialogImpl<CSubLayerDlg>,
	public CDialogHelper<CSubLayerDlg> {
public:
	enum { IDD = IDD_SUBLAYER };

	CSubLayerDlg(WFPEngine& engine, FWPM_SUBLAYER* sublayer);

	BEGIN_MSG_MAP(CSubLayerDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

private:
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	WFPEngine& m_Engine;
	FWPM_SUBLAYER* m_Sublayer;
};
