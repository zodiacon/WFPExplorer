// View.h : interface of the CView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

class CView : public CWindowImpl<CView>
{
public:
	DECLARE_WND_CLASS(nullptr)

	BOOL PreTranslateMessage(MSG* pMsg);

	virtual void OnFinalMessage(HWND /*hWnd*/);

	BEGIN_MSG_MAP(CView)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
};
