#pragma once

#include <FrameView.h>
#include <VirtualListView.h>
#include "Interfaces.h"
#include "resource.h"
#include <ThemeHelper.h>

template<typename T>
class CGenericListViewBase abstract :
	public CFrameView<T, IMainFrame>,
	public CCustomDraw<T>,
	public CVirtualListView<T> {
public:
	DWORD OnPrePaint(int, LPNMCUSTOMDRAW cd) {
		return cd->hdr.hwndFrom == m_List ? CDRF_NOTIFYITEMDRAW : 0;
	}

	DWORD OnItemPrePaint(int, LPNMCUSTOMDRAW cd) {
		return cd->hdr.hwndFrom == m_List ? CDRF_NOTIFYSUBITEMDRAW : 0;
	}

	DWORD OnSubItemPrePaint(int, LPNMCUSTOMDRAW cd) {
		auto p = static_cast<T*>(this);
		auto lv = (LPNMLVCUSTOMDRAW)cd;
		auto const& col = p->GetColumnManager(m_List)->GetColumn(lv->iSubItem);
		if ((col.Flags & ColumnFlags::Numeric) == ColumnFlags::Numeric) {
			::SelectObject(cd->hdc, p->Frame()->GetMonoFont());
		}
		else {
			::SelectObject(cd->hdc, m_List.GetFont());
		}
		return CDRF_NEWFONT;
	}

	LRESULT OnFind(UINT, WPARAM, LPARAM, BOOL&) {
		auto p = static_cast<T*>(this);
		auto findDlg = p->Frame()->GetFindDialog();
		auto searchDown = findDlg->SearchDown();
		int start = m_List.GetNextItem(-1, LVIS_SELECTED);
		CString find(findDlg->GetFindString());
		auto ignoreCase = !findDlg->MatchCase();
		if (ignoreCase)
			find.MakeLower();

		auto columns = m_List.GetHeader().GetItemCount();
		auto count = m_List.GetItemCount();
		int from = searchDown ? start + 1 : start - 1 + count;
		int to = searchDown ? count + start : start + 1;
		int step = searchDown ? 1 : -1;

		int findIndex = -1;
		CString text;
		for (int i = from; i != to; i += step) {
			int index = i % count;
			for (int c = 0; c < columns; c++) {
				m_List.GetItemText(index, c, text);
				if (ignoreCase)
					text.MakeLower();
				if (text.Find(find) >= 0) {
					findIndex = index;
					break;
				}
			}
			if (findIndex >= 0)
				break;
		}

		if (findIndex >= 0) {
			m_List.SelectItem(findIndex);
			m_List.SetFocus();
		}
		else {
			AtlMessageBox(p->m_hWnd, L"Finsihed searching list.", IDS_TITLE, MB_ICONINFORMATION);
		}
		return 0;
	}

	LRESULT OnSave(WORD, WORD, HWND, BOOL&) {
		auto p = static_cast<T*>(this);
		CSimpleFileDialog dlg(FALSE, L"csv", p->GetDefaultSaveFile(), OFN_EXPLORER | OFN_ENABLESIZING | OFN_OVERWRITEPROMPT,
			L"CSV Files (*.csv)\0*.csv\0Text Files (*.txt)\0*.txt\0All Files\0*.*\0", p->m_hWnd);
		ThemeHelper::Suspend();

		auto ok = IDOK == dlg.DoModal();
		ThemeHelper::Resume();
		if (ok && !ListViewHelper::SaveAll(dlg.m_szFileName, m_List, L",")) {
			AtlMessageBox(p->m_hWnd, L"Error in opening file", IDS_TITLE, MB_ICONERROR);
		}
		return 0;
	}

	LRESULT OnFindNext(WORD, WORD, HWND, BOOL&) {
		return static_cast<T*>(this)->SendMessage(CFindReplaceDialog::GetFindReplaceMsg());
	}

	CString GetDefaultSaveFile() const {
		return L"";
	}

protected:
	using BaseFrame = CFrameView<T, IMainFrame>;
	explicit CGenericListViewBase(IMainFrame* frame) : BaseFrame(frame) {}

	BEGIN_MSG_MAP(CGenericListViewBase)
		MESSAGE_HANDLER(CFindReplaceDialog::GetFindReplaceMsg(), OnFind)
		CHAIN_MSG_MAP(CVirtualListView<T>)
		CHAIN_MSG_MAP(CCustomDraw<T>)
		CHAIN_MSG_MAP(BaseFrame)
	ALT_MSG_MAP(1)
		COMMAND_ID_HANDLER(ID_EDIT_FINDNEXT, OnFindNext)
		COMMAND_ID_HANDLER(ID_FILE_SAVE, OnSave)
	END_MSG_MAP()

	CListViewCtrl m_List;
};
