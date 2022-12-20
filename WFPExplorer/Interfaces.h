#pragma once

struct IMainFrame abstract {
	virtual void SetStatusText(int index, PCWSTR text) = 0;
	virtual CUpdateUIBase& UI() = 0;
	virtual HFONT GetMonoFont() const = 0;
	virtual bool TrackPopupMenu(HMENU hMenu, DWORD flags, int x, int y, HWND hWnd = nullptr) = 0;
	virtual CFindReplaceDialog* GetFindDialog() const = 0;
};

