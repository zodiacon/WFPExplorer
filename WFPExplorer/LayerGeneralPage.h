#pragma once

#include <DialogHelper.h>
#include "resource.h"

class WFPEngine;
struct WFPLayerInfo;

class CLayerGeneralPage :
	public CPropertyPageImpl<CLayerGeneralPage>,
	public CDialogHelper<CLayerGeneralPage> {
public:
	enum { IDD = IDD_LAYERINFO };

	CLayerGeneralPage(WFPEngine& engine, WFPLayerInfo const& layer);

	BEGIN_MSG_MAP(CLayerGeneralPage)
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
	WFPLayerInfo const& m_Layer;
};
