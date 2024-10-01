#include "pch.h"
#include "resource.h"
#include "SubLayerDlg.h"
#include "WFPHelper.h"
#include <WFPEngine.h>
#include "StringHelper.h"

CSubLayerDlg::CSubLayerDlg(WFPEngine& engine, FWPM_SUBLAYER* sublayer) : m_Engine(engine), m_Sublayer(sublayer) {
}

LRESULT CSubLayerDlg::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
	SetDialogIcon(IDI_SUBLAYER);
	SetWindowText(std::format(L"Sublayer ({})", m_Sublayer->displayData.name).c_str());
	SetDlgItemText(IDC_KEY, StringHelper::GuidToString(m_Sublayer->subLayerKey));
	SetDlgItemText(IDC_NAME, StringHelper::ParseMUIString(m_Sublayer->displayData.name));
	SetDlgItemText(IDC_DESC, m_Sublayer->displayData.description);
	SetDlgItemInt(IDC_WEIGHT, m_Sublayer->weight);

	if (m_Sublayer->providerKey) {
		SetDlgItemText(IDC_PROVIDER, WFPHelper::GetProviderName(m_Engine, *m_Sublayer->providerKey));
	}
	else {
		GetDlgItem(IDC_PROVIDER_PROP).EnableWindow(FALSE);
	}
	 
	CheckDlgButton(IDC_PERSISTENT, (m_Sublayer->flags & FWPM_SUBLAYER_FLAG_PERSISTENT) ? BST_CHECKED : BST_UNCHECKED);

	return 0;
}

LRESULT CSubLayerDlg::OnCloseCmd(WORD, WORD wID, HWND, BOOL&) {
	EndDialog(wID);
	return 0;
}

LRESULT CSubLayerDlg::OnProviderProperties(WORD, WORD wID, HWND, BOOL&) {
	ATLASSERT(m_Sublayer->providerKey);
	WFPHelper::ShowProviderProperties(m_Engine, *m_Engine.GetProviderByKey(*m_Sublayer->providerKey));

	return 0;
}
