#pragma once

static const UINT WM_UPDATE_DARKMODE = WM_APP + 56;

struct IMainFrame abstract {
	virtual void SetStatusText(int index, PCWSTR text) = 0;
	virtual CUpdateUIBase& UI() = 0;
	virtual HFONT GetMonoFont() const = 0;
	virtual bool TrackPopupMenu(HMENU hMenu, DWORD flags, int x, int y, HWND hWnd = nullptr) = 0;
	virtual CFindReplaceDialog* GetFindDialog() const = 0;
	// callout: a callout of the layer to select and bring into view
	virtual void ShowLayerMap(GUID const& layer, GUID const& callout = GUID_NULL) = 0;
	virtual void ShowCalloutMap(GUID const& callout) = 0;
	virtual void SetViewTitle(HWND hView, PCWSTR title) = 0;	// the title of the view's tab
};

