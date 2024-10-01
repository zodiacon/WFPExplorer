#pragma once

#include <DialogHelper.h>

class CNewSubLayerDlg :
	public CDialogImpl<CNewSubLayerDlg>,
	public CDialogHelper<CNewSubLayerDlg> {
public:
	enum { IDD = IDD_NEWSUBLAYER };

	explicit CNewSubLayerDlg(FWPM_SUBLAYER* sublayer);

	bool IsNewSubLayer() const;

	BEGIN_MSG_MAP(CNewSubLayerDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_ID_HANDLER(IDC_GENERATE, OnGenerateKey)
		COMMAND_ID_HANDLER(IDC_PROVIDER_PROP, OnProviderProperties)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

private:
	bool UpdateSubLayer();
	LRESULT ReportOpenEngineFailure(PCWSTR errorText = nullptr) const;

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnGenerateKey(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnProviderProperties(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	FWPM_SUBLAYER* m_SubLayer;
	CString m_Name, m_Desc;
	GUID m_ProviderKey;
};
