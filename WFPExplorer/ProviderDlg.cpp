#include "pch.h"
#include "resource.h"
#include "ProviderDlg.h"
#include "StringHelper.h"

CProviderDlg::CProviderDlg(FWPM_PROVIDER* provider) : m_Provider(provider) {
}

LRESULT CProviderDlg::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
    SetDialogIcon((m_Provider->flags & FWPM_PROVIDER_FLAG_PERSISTENT) ? IDI_PROVIDER_PERSISTENT : IDI_PROVIDER);
    if (m_Provider->providerKey != GUID_NULL) {
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
    }
    else {
        UINT ids[] = { IDC_KEY, IDC_NAME, IDC_DESC, IDC_SERVICENAME };
        for (auto id : ids) {
            ((CEdit)GetDlgItem(id)).SetReadOnly(FALSE);
        }
        GetDlgItem(IDC_PERSISTENT).EnableWindow();
        GetDlgItem(IDC_DISABLED).EnableWindow();
        GetDlgItem(IDC_GENERATE).ShowWindow(SW_SHOW);
        GetDlgItem(IDOK).SetWindowTextW(L"Add");
    }
    return 0;
}

LRESULT CProviderDlg::OnCloseCmd(WORD, WORD id, HWND, BOOL&) {
    if (m_Provider->providerKey == GUID_NULL) {
        CString key;
        GetDlgItem(IDC_KEY).GetWindowText(key);
        ::CLSIDFromString(key, &m_Provider->providerKey);
        GetDlgItemText(IDC_NAME, m_Name);
        if (!m_Name.IsEmpty())
            m_Provider->displayData.name = (PWSTR)(PCWSTR)m_Name;
        GetDlgItemText(IDC_DESC, m_Desc);
        if (!m_Desc.IsEmpty())
            m_Provider->displayData.description = (PWSTR)(PCWSTR)m_Desc;
        GetDlgItemText(IDC_SERVICENAME, m_Service);
        if (!m_Service.IsEmpty())
            m_Provider->serviceName = (PWSTR)(PCWSTR)m_Service;

        if (IsDlgButtonChecked(IDC_PERSISTENT) == BST_CHECKED)
            m_Provider->flags |= FWPM_PROVIDER_FLAG_PERSISTENT;
        if (IsDlgButtonChecked(IDC_DISABLED) == BST_CHECKED)
            m_Provider->flags |= FWPM_PROVIDER_FLAG_DISABLED;
    }
    EndDialog(id);
    return 0;
}

LRESULT CProviderDlg::OnGenerateKey(WORD, WORD wID, HWND, BOOL&) {
    GUID guid;
    ::CoCreateGuid(&guid);
    WCHAR sguid[64];
    ::StringFromGUID2(guid, sguid, _countof(sguid));
    SetDlgItemText(IDC_KEY, sguid);

    return 0;
}
