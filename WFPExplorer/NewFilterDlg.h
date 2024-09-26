#pragma once

#include <DialogHelper.h>

class CNewFilterDlg :
	public CDialogImpl<CNewFilterDlg>,
	public CDialogHelper<CNewFilterDlg> {
public:
	enum { IDD = IDD_NEWFILTER };

	explicit CNewFilterDlg(FWPM_FILTER* filter);

	bool IsNewFilter() const;

	BEGIN_MSG_MAP(CNewFilterDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_ID_HANDLER(IDC_GENERATE, OnGenerateKey)
		COMMAND_ID_HANDLER(IDC_LAYER_PROP, OnLayerProperties)
		COMMAND_ID_HANDLER(IDC_SUBLAYER_PROP, OnSublayerProperties)
		COMMAND_ID_HANDLER(IDC_PROVIDER_PROP, OnProviderProperties)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

private:
	LRESULT ReportOpenEngineFailure(PCWSTR errorText = nullptr) const;

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnGenerateKey(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnLayerProperties(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSublayerProperties(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnProviderProperties(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	FWPM_FILTER* m_Filter;
	CString m_Name, m_Desc;
	GUID m_ProviderKey;
	UINT64 m_Weight;
};
