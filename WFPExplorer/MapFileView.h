#pragma once

#include <FrameView.h>
#include <NodeGraphControl.h>
#include <WTLHelper.h>
#include "Interfaces.h"
#include "resource.h"

//
// a map saved to a file (File > Open): the graph as it was saved, with its tooltips;
// not connected to the engine, so there are no properties to show
//
class CMapFileView : public CFrameView<CMapFileView, IMainFrame> {
public:
	explicit CMapFileView(IMainFrame* frame);

	bool Load(PCWSTR path);		// once created
	CString GetTitle() const;

	BEGIN_MSG_MAP(CMapFileView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WTLHelper::ThemeChangedMessage, OnThemeChanged)
		CHAIN_MSG_MAP(BaseFrame)
	ALT_MSG_MAP(1)
		COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnRefresh)
		COMMAND_ID_HANDLER(ID_FILE_SAVE, OnSave)
	END_MSG_MAP()

private:
	void UpdateBackground();

	LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnThemeChanged(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnRefresh(WORD, WORD, HWND, BOOL&);
	LRESULT OnSave(WORD, WORD, HWND, BOOL&);

	NodeGraphCtrl::CNodeGraphControl m_Graph;
	CString m_Path;
	bool m_NeedFit{ false };	// fit the graph in view once it has a size
};
