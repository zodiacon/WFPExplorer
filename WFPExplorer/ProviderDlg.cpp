#include "pch.h"
#include "resource.h"
#include "ProviderDlg.h"
#include "StringHelper.h"

CProviderDlg::CProviderDlg(FWPM_PROVIDER* provider) : m_Provider(provider) {
}

LRESULT CProviderDlg::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
    SetDialogIcon((m_Provider->flags & FWPM_PROVIDER_FLAG_PERSISTENT) ? IDI_PROVIDER_PERSISTENT : IDI_PROVIDER);
    SetDlgItemText(IDC_KEY, StringHelper::GuidToString(m_Provider->providerKey));
    auto name = StringHelper::ParseMUIString(m_Provider->displayData.name);
    SetDlgItemText(IDC_NAME, name);
    if (!name.IsEmpty() && name[0] != L'@')
        SetWindowText(L"Provider: " + name);
    SetDlgItemText(IDC_DESC, StringHelper::ParseMUIString(m_Provider->displayData.description));
    SetDlgItemText(IDC_SERVICENAME, m_Provider->serviceName);

    if (m_Provider->flags & FWPM_PROVIDER_FLAG_PERSISTENT)
        CheckDlgButton(IDC_PERSISTENT, BST_CHECKED);
    if (m_Provider->flags & FWPM_PROVIDER_FLAG_DISABLED)
        CheckDlgButton(IDC_DISABLED, BST_CHECKED);

    if (m_Provider->providerData.size) {
        SetDlgItemText(IDC_DATASIZE, std::format(L"Provider data size: {} bytes", m_Provider->providerData.size).c_str());
        SetDlgItemText(IDC_PROVIDERDATA, StringHelper::FormatBinary(m_Provider->providerData.data, m_Provider->providerData.size));
    }
    else {
        GetDlgItem(IDC_PROVIDERDATA).ShowWindow(SW_HIDE);
    }

    return 0;
}

LRESULT CProviderDlg::OnCloseCmd(WORD, WORD wID, HWND, BOOL&) {
    EndDialog(wID);
    return 0;
}
