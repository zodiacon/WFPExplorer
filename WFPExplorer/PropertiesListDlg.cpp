#include "pch.h"
#include "PropertiesListDlg.h"
#include <ClipboardHelper.h>

CPropertiesListDlg::CPropertiesListDlg(CString title, UINT icon, Properties properties) :
	m_Title(std::move(title)), m_Icon(icon), m_Properties(std::move(properties)) {
}

LRESULT CPropertiesListDlg::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
	InitDynamicLayout(false);
	SetWindowText(m_Title);
	SetDialogIcon(m_Icon);

	m_List.Attach(GetDlgItem(IDC_LIST));
	m_List.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP);
	m_List.InsertColumn(0, L"Property", LVCFMT_LEFT, 150);
	m_List.InsertColumn(1, L"Value", LVCFMT_LEFT, 300);
	for (int i = 0; i < (int)m_Properties.size(); i++) {
		m_List.InsertItem(i, m_Properties[i].first);
		m_List.SetItemText(i, 1, m_Properties[i].second);
	}
	// the values get the rest of the width
	m_List.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);

	CenterWindow(GetParent());
	return TRUE;
}

LRESULT CPropertiesListDlg::OnClose(WORD, WORD id, HWND, BOOL&) {
	EndDialog(id);
	return 0;
}

//
// one "name<tab>value" line per property
//
LRESULT CPropertiesListDlg::OnCopy(WORD, WORD, HWND, BOOL&) {
	CString text;
	for (auto& [name, value] : m_Properties)
		text += name + L"\t" + value + L"\r\n";
	ClipboardHelper::CopyText(m_hWnd, text);
	return 0;
}
