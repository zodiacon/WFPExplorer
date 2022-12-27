#include "pch.h"
#include "FilterGeneralPage.h"
#include "StringHelper.h"
#include <WFPEngine.h>
#include "WFPHelper.h"

CFilterGeneralPage::CFilterGeneralPage(WFPEngine& engine, FWPM_FILTER* filter) : m_Engine(engine), m_Filter(filter) {
}

LRESULT CFilterGeneralPage::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
    SetDlgItemText(IDC_KEY, StringHelper::GuidToString(m_Filter->filterKey));
    SetDlgItemText(IDC_ID, std::to_wstring(m_Filter->filterId).c_str());
    SetDlgItemText(IDC_PROVIDER, m_Filter->providerKey ? WFPHelper::GetProviderName(m_Engine, *m_Filter->providerKey) : CString(L""));
    SetDlgItemText(IDC_NAME, StringHelper::ParseMUIString(m_Filter->displayData.name));
    SetDlgItemText(IDC_DESC, StringHelper::ParseMUIString(m_Filter->displayData.description));
    SetDlgItemText(IDC_LAYER, WFPHelper::GetLayerName(m_Engine, m_Filter->layerKey));
    SetDlgItemText(IDC_SUBLAYER, WFPHelper::GetSublayerName(m_Engine, m_Filter->subLayerKey));
    SetDlgItemText(IDC_ACTIONTYPE, StringHelper::WFPFilterActionTypeToString(m_Filter->action.type));
    if (m_Filter->action.type & FWP_ACTION_FLAG_CALLOUT) {
        SetDlgItemText(IDC_CALLOUT_OR_FILTER, WFPHelper::GetCalloutName(m_Engine, m_Filter->action.calloutKey));
        AddIconToButton(IDC_CALLOUT_PROP, IDI_CALLOUT);
    }
    else {
        GetDlgItem(IDC_COFI).ShowWindow(SW_HIDE);
        GetDlgItem(IDC_CALLOUT_OR_FILTER).ShowWindow(SW_HIDE);
        GetDlgItem(IDC_CALLOUT_PROP).ShowWindow(SW_HIDE);
    }
    SetDlgItemText(IDC_WEIGHT, StringHelper::WFPValueToString(m_Filter->weight, true));
    SetDlgItemText(IDC_EFFECTIVEWEIGHT, StringHelper::WFPValueToString(m_Filter->effectiveWeight, true));

    auto flags = StringHelper::WFPFilterFlagsToString(m_Filter->flags);
    if (!flags.IsEmpty())
        flags = std::format(L"0x{:X} ({})", (UINT32)m_Filter->flags, (PCWSTR)flags).c_str();
    else
        flags = L"(None)";
    SetDlgItemText(IDC_FLAGS, flags);

    AddIconToButton(IDC_PROVIDER_PROP, IDI_PROVIDER);
    AddIconToButton(IDC_LAYER_PROP, IDI_LAYERS);
    AddIconToButton(IDC_SUBLAYER_PROP, IDI_SUBLAYER);

    if (m_Filter->providerKey == nullptr) {
        GetDlgItem(IDC_PROVIDER_PROP).EnableWindow(FALSE);
    }

    return 0;
}

LRESULT CFilterGeneralPage::OnShowLayer(WORD, WORD, HWND, BOOL&) {
    auto layer = m_Engine.GetLayerByKey(m_Filter->layerKey);
    ATLASSERT(layer);
    if (!layer)
        AtlMessageBox(m_hWnd, L"Layer not found", IDS_TITLE, MB_ICONERROR);
    else
        WFPHelper::ShowLayerProperties(m_Engine, *layer);

    return 0;
}

LRESULT CFilterGeneralPage::OnShowProvider(WORD, WORD, HWND, BOOL&) {
    auto provider = m_Engine.GetProviderByKey(*m_Filter->providerKey);
    if (!provider)
        AtlMessageBox(m_hWnd, L"Provider not found", IDS_TITLE, MB_ICONERROR);
    else
        WFPHelper::ShowProviderProperties(m_Engine, *provider);
    return 0;
}

LRESULT CFilterGeneralPage::OnShowSublayer(WORD, WORD, HWND, BOOL&) {
    return LRESULT();
}

LRESULT CFilterGeneralPage::OnShowCallout(WORD, WORD, HWND, BOOL&) {
    auto callout = m_Engine.GetCalloutByKey(m_Filter->action.calloutKey);
    ATLASSERT(callout);
    WFPHelper::ShowCalloutProperties(m_Engine, *callout);

    return 0;
}
