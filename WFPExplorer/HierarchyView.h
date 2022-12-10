#pragma once

#include <FrameView.h>
#include <TreeViewHelper.h>
#include <VirtualListView.h>
#include "Interfaces.h"
#include <WFPEngine.h>
#include <CustomSplitterWindow.h>
#include "resource.h"

class WFPEngine;
class CLayersView;
class CFiltersView;
class CCalloutsView;

class CHierarchyView :
	public CFrameView<CHierarchyView, IMainFrame>,
	public CTreeViewHelper<CHierarchyView> {
public:
	CHierarchyView(IMainFrame* frame, WFPEngine& engine);

	void Refresh();
	void OnTreeSelChanged(HWND tree, HTREEITEM hOld, HTREEITEM hNew);
	bool OnTreeDoubleClick(HWND tree, HTREEITEM hItem);

	BEGIN_MSG_MAP(CHierarchyView)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		CHAIN_MSG_MAP(CTreeViewHelper<CHierarchyView>)
		CHAIN_MSG_MAP(BaseFrame)
	ALT_MSG_MAP(1)
		COMMAND_ID_HANDLER(ID_EDIT_PROPERTIES, OnProperties)
		COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnRefresh)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

private:
	enum class TreeItemType {
		None, Layers, Layer, Filters, Callouts, Filter, Callout,
	};

	void BuildTree();
	void UpdateUI();
	bool ShowProperties(HTREEITEM hItem);

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	WFPEngine& m_Engine;

	CTreeViewCtrl m_Tree;
	CCustomSplitterWindow m_Splitter;
	CLayersView* m_LayersView;
	CFiltersView* m_FiltersView;
	CCalloutsView* m_CalloutsView;
	std::unordered_map<HTREEITEM, GUID> m_LayersMap;
	std::unordered_map<HTREEITEM, GUID> m_FiltersMap;
	std::unordered_map<HTREEITEM, GUID> m_CalloutsMap;
};
