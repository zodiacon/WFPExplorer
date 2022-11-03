// aboutdlg.cpp : implementation of the CAboutDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "resource.h"
#include "AboutDlg.h"
#include <VersionResourceHelper.h>

LRESULT CAboutDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	SetDialogIcon(IDR_MAINFRAME);
	CenterWindow(GetParent());
	VersionResourceHelper vh;
	auto version = vh.GetValue(L"ProductVersion");
	SetDlgItemText(IDC_VERSION, vh.GetValue(L"ProductName") + L" v" + version);
	SetDlgItemText(IDC_COPYRIGHT, vh.GetValue(L"LegalCopyright"));

	return TRUE;
}

LRESULT CAboutDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	EndDialog(wID);
	return 0;
}
