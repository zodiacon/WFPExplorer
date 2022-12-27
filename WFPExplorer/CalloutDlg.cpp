#include "pch.h"
#include "resource.h"
#include "CalloutDlg.h"
#include "StringHelper.h"
#include <WFPEngine.h>
#include "WFPHelper.h"

CCalloutDlg::CCalloutDlg(WFPEngine& engine, FWPM_CALLOUT* callout) : m_Engine(engine), m_Callout(callout) {
}

LRESULT CCalloutDlg::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
    SetWindowText(L"Callout: " + WFPHelper::GetCalloutName(m_Engine, m_Callout->calloutKey));
    SetDialogIcon(IDI_CALLOUT);
    SetDlgItemText(IDC_KEY, StringHelper::GuidToString(m_Callout->calloutKey));
    SetDlgItemText(IDC_NAME, StringHelper::ParseMUIString(m_Callout->displayData.name));
    SetDlgItemText(IDC_DESC, StringHelper::ParseMUIString(m_Callout->displayData.description));
    SetDlgItemInt(IDC_ID, m_Callout->calloutId);
    SetDlgItemText(IDC_LAYER, WFPHelper::GetLayerName(m_Engine, m_Callout->applicableLayer));

    AddIconToButton(IDC_LAYER_PROP, IDI_LAYERS);
    if (m_Callout->providerKey) {
        SetDlgItemText(IDC_PROVIDER, WFPHelper::GetProviderName(m_Engine, *m_Callout->providerKey));
        AddIconToButton(IDC_PROVIDER_PROP, IDI_PROVIDER);
    }
    else {
        GetDlgItem(IDC_PROVIDER_PROP).ShowWindow(SW_HIDE);
    }

    if (m_Callout->flags & FWPM_CALLOUT_FLAG_PERSISTENT)
        CheckDlgButton(IDC_PERSISTENT, BST_CHECKED);
    if (m_Callout->flags & FWPM_CALLOUT_FLAG_REGISTERED)
        CheckDlgButton(IDC_REGISTERED, BST_CHECKED);
    if (m_Callout->flags & FWPM_CALLOUT_FLAG_USES_PROVIDER_CONTEXT)
        CheckDlgButton(IDC_PROVIDER_CONTEXT, BST_CHECKED);

    auto layer = m_Engine.GetLayerByKey(m_Callout->applicableLayer);

    if (m_Callout->providerData.size) {
        SetDlgItemText(IDC_DATASIZE, std::format(L"Provider data size: {} bytes", m_Callout->providerData.size).c_str());
        SetDlgItemText(IDC_PROVIDERDATA, StringHelper::FormatBinary(m_Callout->providerData.data, m_Callout->providerData.size, 16, true));
    }
    else {
        GetDlgItem(IDC_PROVIDERDATA).ShowWindow(SW_HIDE);
    }

    return 0;
}

LRESULT CCalloutDlg::OnCloseCmd(WORD, WORD wID, HWND, BOOL&) {
    EndDialog(wID);
    return 0;
}

LRESULT CCalloutDlg::OnShowProvider(WORD, WORD, HWND, BOOL&) {
    auto provider = m_Engine.GetProviderByKey(*m_Callout->providerKey);
    WFPHelper::ShowProviderProperties(m_Engine, *provider);

    return 0;
}

LRESULT CCalloutDlg::OnShowLayer(WORD, WORD, HWND, BOOL&) {
    auto layer = m_Engine.GetLayerByKey(m_Callout->applicableLayer);
    WFPHelper::ShowLayerProperties(m_Engine, *layer);

    return 0;
}
