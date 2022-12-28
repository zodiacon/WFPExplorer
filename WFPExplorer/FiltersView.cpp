#include "pch.h"
#include "FiltersView.h"
#include <WFPEngine.h>
#include "StringHelper.h"
#include <SortHelper.h>
#include "resource.h"
#include "FilterGeneralPage.h"
#include "FilterConditionsPage.h"
#include "WFPHelper.h"
#include <ResizablePropertySheet.h>
#include "AppSettings.h"
#include <WFPEnumerators.h>
#include <ClipboardHelper.h>
#include <fstream>
#include <ThemeHelper.h>

CFiltersView::CFiltersView(IMainFrame* frame, WFPEngine& engine) : CGenericListViewBase(frame), m_Engine(engine) {
}

void CFiltersView::SetLayer(GUID const& layer) {
	m_Layer = layer;
}

void CFiltersView::Refresh() {
	CWaitCursor wait;
	WFPFilterEnumerator enumerator(m_Engine.Handle());
	if (m_Layer == GUID_NULL)
		m_Filters = enumerator.Next<FilterInfo>(8192);
	else
		m_Filters = enumerator.NextFiltered<FilterInfo>([&](auto& filter) { return filter->layerKey == m_Layer; }, 8192);
	Sort(m_List);
	m_List.SetItemCountEx((int)m_Filters.size(), LVSICF_NOSCROLL);
	Frame()->SetStatusText(3, std::format(L"{} Filters", m_Filters.size()).c_str());
}

CString CFiltersView::GetColumnText(HWND, int row, int col) {
	auto& fi = m_Filters[row];
	auto info = fi.Data;
	switch (GetColumnManager(m_List)->GetColumnTag<ColumnType>(col)) {
		case ColumnType::Key: return StringHelper::GuidToString(info->filterKey);
		case ColumnType::Name: return fi.Name();
		case ColumnType::Desc: return fi.Desc();
		case ColumnType::Id: return std::format(L"0x{:X}", info->filterId).c_str();
		case ColumnType::ConditionCount: return std::to_wstring(info->numFilterConditions).c_str();
		case ColumnType::LayerKey: return StringHelper::GuidToString(info->layerKey);
		case ColumnType::SubLayerKey: return StringHelper::GuidToString(info->subLayerKey);
		case ColumnType::Weight: return StringHelper::WFPValueToString(info->weight, true);
		case ColumnType::Action: return StringHelper::WFPFilterActionTypeToString(info->action.type);
		case ColumnType::ProviderData: return info->providerData.size == 0 ? L"" : std::format(L"{} Bytes", info->providerData.size).c_str();
		case ColumnType::ActionKey:
			if (fi.FilterAction.IsEmpty()) {
				if (info->action.calloutKey == GUID_NULL)
					fi.FilterAction = L"(None)";
				else {
					auto filter = m_Engine.GetFilterByKey(info->action.filterType);
					if (filter)
						fi.FilterAction = StringHelper::ParseMUIString(filter->displayData.name);
					else {
						auto callout = m_Engine.GetCalloutByKey(info->action.calloutKey);
						if (callout)
							fi.FilterAction = callout->displayData.name;
					}
				}
			}
			return fi.FilterAction;

		case ColumnType::Flags:
			if (info->flags == 0)
				return L"0";
			return std::format(L"0x{:02X} ({})", info->flags,
				(PCWSTR)StringHelper::WFPFilterFlagsToString(info->flags)).c_str();
		case ColumnType::EffectiveWeight: return StringHelper::WFPValueToString(info->effectiveWeight, true);
		case ColumnType::ProviderName: return GetProviderName(fi);
		case ColumnType::Layer: return GetLayerName(fi);
		case ColumnType::SubLayer: return GetSublayerName(fi);
	}
	return CString();
}

void CFiltersView::UpdateUI() const {
	auto& ui = Frame()->UI();
	auto selected = m_List.GetSelectedCount();
	ui.UIEnable(ID_EDIT_PROPERTIES, selected == 1);
	ui.UIEnable(ID_EDIT_DELETE, selected > 0);
	ui.UIEnable(ID_EDIT_COPY, selected > 0);
	ui.UIEnable(ID_EDIT_FINDNEXT, Frame()->GetFindDialog() != nullptr);
}

CString const& CFiltersView::GetProviderName(FilterInfo& info) const {
	if (info.ProviderName.IsEmpty() && info.Data->providerKey)
		info.ProviderName = WFPHelper::GetProviderName(m_Engine, *info.Data->providerKey);
	return info.ProviderName;
}

CString const& CFiltersView::GetLayerName(FilterInfo& info) const {
	if (info.Layer.IsEmpty() && info.Data->layerKey != GUID_NULL) {
		auto layer = m_Engine.GetLayerByKey(info.Data->layerKey);
		if (layer && layer->displayData.name && layer->displayData.name[0] != L'@')
			info.Layer = layer->displayData.name;
		else
			info.Layer = StringHelper::GuidToString(info.Data->layerKey);
	}
	return info.Layer;
}

CString const& CFiltersView::GetSublayerName(FilterInfo& info) const {
	if (info.SubLayer.IsEmpty()) {
		auto layer = m_Engine.GetSublayerByKey(info.Data->subLayerKey);
		if (layer)
			info.SubLayer = StringHelper::ParseMUIString(layer->displayData.name);
		else
			info.SubLayer = StringHelper::GuidToString(info.Data->layerKey);
	}
	return info.SubLayer;
}

LRESULT CFiltersView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	m_hWndClient = m_List.Create(m_hWnd, rcDefault, nullptr,
		WS_CHILD | WS_VISIBLE | LVS_OWNERDATA | LVS_REPORT | LVS_SHOWSELALWAYS);
	m_List.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_HEADERDRAGDROP);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"Filter Key", 0, 300, ColumnType::Key, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Filter Name", 0, 170, ColumnType::Name);
	cm->AddColumn(L"Layer", LVCFMT_LEFT, 200, ColumnType::Layer);
	cm->AddColumn(L"Weight", LVCFMT_RIGHT, 140, ColumnType::Weight, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Effective Weight", LVCFMT_RIGHT, 140, ColumnType::EffectiveWeight, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Filter Id", LVCFMT_RIGHT, 100, ColumnType::Id, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Conditions", LVCFMT_RIGHT, 70, ColumnType::ConditionCount, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Action", LVCFMT_LEFT, 110, ColumnType::Action);
	cm->AddColumn(L"Action Filter/Callout", LVCFMT_LEFT, 120, ColumnType::ActionKey);
	cm->AddColumn(L"Flags", LVCFMT_LEFT, 150, ColumnType::Flags);
	cm->AddColumn(L"Provider Data", LVCFMT_RIGHT, 100, ColumnType::ProviderData);
	cm->AddColumn(L"Description", 0, 180, ColumnType::Desc);
	cm->AddColumn(L"Provider", 0, 240, ColumnType::ProviderName);
	cm->AddColumn(L"Sublayer", LVCFMT_LEFT, 250, ColumnType::SubLayer);

	CImageList images;
	images.Create(16, 16, ILC_COLOR32 | ILC_MASK, 1, 1);
	UINT icons[] = { IDI_FILTER, IDI_FILTER_PERMIT, IDI_FILTER_BLOCK, IDI_FILTER_REFRESH, IDI_CALLOUT };
	for (auto icon : icons)
		images.AddIcon(AtlLoadIconImage(icon, 0, 16, 16));
	m_List.SetImageList(images, LVSIL_SMALL);

	return 0;
}

LRESULT CFiltersView::OnRefresh(WORD, WORD, HWND, BOOL&) {
	Refresh();
	return 0;
}

LRESULT CFiltersView::OnProperties(WORD, WORD, HWND, BOOL&) {
	ATLASSERT(m_List.GetSelectedCount() == 1);
	auto& filter = m_Filters[m_List.GetNextItem(-1, LVIS_SELECTED)];
	WFPHelper::ShowFilterProperties(m_Engine, filter.Data);

	return 0;
}

LRESULT CFiltersView::OnActivate(UINT, WPARAM activate, LPARAM, BOOL&) {
	if (activate)
		UpdateUI();
	return 0;
}

LRESULT CFiltersView::OnDeleteFilter(WORD, WORD, HWND, BOOL&) {
	int selected = m_List.GetSelectedCount();
	ATLASSERT(selected > 0);
	if (AtlMessageBox(m_hWnd, L"Delete selected filter(s)?", IDS_TITLE, MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2) == IDNO)
		return 0;

	int deleted = 0;
	for (auto index : SelectedItemsView(m_List)) {
		auto const& fi = m_Filters[index];
		if (m_Engine.DeleteFilter(fi.Data->filterKey))
			deleted++;
	}
	if (deleted > 0) {
		m_List.SelectAllItems(false);
		Refresh();
	}
	if (deleted < selected)
		AtlMessageBox(m_hWnd, std::format(L"Deleted {}/{} filters", deleted, selected).c_str(), IDS_TITLE, MB_ICONINFORMATION);

	return 0;
}

LRESULT CFiltersView::OnCopy(WORD, WORD, HWND, BOOL&) {
	auto text = ListViewHelper::GetSelectedRowsAsString(m_List, L",");
	ClipboardHelper::CopyText(m_hWnd, text);

	return 0;
}

void CFiltersView::DoSort(SortInfo const* si) {
	auto col = GetColumnManager(m_List)->GetColumnTag<ColumnType>(si->SortColumn);
	auto asc = si->SortAscending;

	auto compare = [&](auto& filter1, auto& filter2) {
		auto f1 = filter1.Data, f2 = filter2.Data;
		switch (col) {
			case ColumnType::Key: return SortHelper::Sort(StringHelper::GuidToString(f1->filterKey), StringHelper::GuidToString(f2->filterKey), asc);
			case ColumnType::Name: return SortHelper::Sort(filter1.Name(), filter2.Name(), asc);
			case ColumnType::Desc: return SortHelper::Sort(filter1.Desc(), filter2.Desc(), asc);
			case ColumnType::Id: return SortHelper::Sort(f1->filterId, f2->filterId, asc);
			case ColumnType::Action: return SortHelper::Sort(f1->action.type, f2->action.type, asc);
			case ColumnType::ActionKey: return SortHelper::Sort(filter1.FilterAction, filter2.FilterAction, asc);
			case ColumnType::Flags: return SortHelper::Sort(f1->flags, f2->flags, asc);
			case ColumnType::ProviderName: return SortHelper::Sort(GetProviderName(filter1), GetProviderName(filter2), asc);
			case ColumnType::Weight: return SortHelper::Sort(*f1->weight.uint64, *f2->weight.uint64, asc);
			case ColumnType::EffectiveWeight: return SortHelper::Sort(*f1->effectiveWeight.uint64, *f2->effectiveWeight.uint64, asc);
			case ColumnType::Layer: return SortHelper::Sort(GetLayerName(filter1), GetLayerName(filter2), asc);
			case ColumnType::SubLayer: return SortHelper::Sort(GetSublayerName(filter1), GetSublayerName(filter2), asc);
			case ColumnType::ConditionCount: return SortHelper::Sort(f1->numFilterConditions, f2->numFilterConditions, asc);
			case ColumnType::ProviderData: return SortHelper::Sort(f1->providerData.size, f2->providerData.size, asc);
		}
		return false;
	};
	std::ranges::sort(m_Filters, compare);
}

int CFiltersView::GetSaveColumnRange(HWND, int&) const {
	return 1;
}

int CFiltersView::GetRowImage(HWND, int row, int) const {
	auto& filter = m_Filters[row];
	switch (filter.Data->action.type) {
		case FWP_ACTION_BLOCK: return 2;
		case FWP_ACTION_PERMIT: return 1;
		case FWP_ACTION_CONTINUE: return 3;
	}
	return 4;
}

void CFiltersView::OnStateChanged(HWND, int from, int to, UINT oldState, UINT newState) {
	if ((newState & LVIS_SELECTED) || (oldState & LVIS_SELECTED))
		UpdateUI();
}

bool CFiltersView::OnDoubleClickList(HWND, int row, int col, POINT const& pt) {
	LRESULT result;
	return ProcessWindowMessage(m_hWnd, WM_COMMAND, ID_EDIT_PROPERTIES, 0, result, 1);
}

bool CFiltersView::OnRightClickList(HWND, int row, int col, POINT const& pt) {
	if (row < 0)
		return false;

	CMenu menu;
	menu.LoadMenu(IDR_CONTEXT);

	return Frame()->TrackPopupMenu(menu.GetSubMenu(0), 0, pt.x, pt.y);
}

CString CFiltersView::GetDefaultSaveFile() const {
	return L"Filters";
}

CString const& CFiltersView::FilterInfo::Name() const {
	if (m_Name.IsEmpty())
		m_Name = StringHelper::ParseMUIString(Data->displayData.name);
	return m_Name;
}

CString const& CFiltersView::FilterInfo::Desc() const {
	if (m_Desc.IsEmpty())
		m_Desc = StringHelper::ParseMUIString(Data->displayData.description);
	return m_Desc;
}
