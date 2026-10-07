#pragma once

#include <DialogHelper.h>
#include "resource.h"

//
// a list of name/value pairs in a resizable dialog, with a button that copies them as text
// (for objects that have more properties than a view has columns, e.g. a network event)
//
class CPropertiesListDlg :
	public CDialogImpl<CPropertiesListDlg>,
	public CDynamicDialogLayout<CPropertiesListDlg>,
	public CDialogHelper<CPropertiesListDlg> {
public:
	enum { IDD = IDD_PROPERTIESLIST };

	using Properties = std::vector<std::pair<CString, CString>>;
	CPropertiesListDlg(CString title, UINT icon, Properties properties);

	BEGIN_MSG_MAP(CPropertiesListDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnClose)
		COMMAND_ID_HANDLER(IDCANCEL, OnClose)
		COMMAND_ID_HANDLER(IDC_COPY, OnCopy)
		CHAIN_MSG_MAP(CDynamicDialogLayout<CPropertiesListDlg>)
	END_MSG_MAP()

private:
	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnClose(WORD, WORD id, HWND, BOOL&);
	LRESULT OnCopy(WORD, WORD, HWND, BOOL&);

	CString m_Title;
	UINT m_Icon;
	Properties m_Properties;
	CListViewCtrl m_List;
};
