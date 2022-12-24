#include "pch.h"
#include "CalloutsView.h"
#include "StringHelper.h"
#include <SortHelper.h>
#include "resource.h"
#include <ranges>
#include <ClipboardHelper.h>

CCalloutsView::CCalloutsView(IMainFrame* frame, WFPEngine& engine) : CGenericListViewBase(frame), m_Engine(engine) {
}

LRESULT CCalloutsView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	m_hWndClient = m_List.Create(m_hWnd, rcDefault, nullptr,
		WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | LVS_OWNERDATA | LVS_REPORT | LVS_SHOWSELALWAYS);
	m_List.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"Callout Key", 0, 300, ColumnType::Key, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Layer ID", LVCFMT_RIGHT, 90, ColumnType::Id, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Name", 0, 250, ColumnType::Name);
	cm->AddColumn(L"Flags", 0, 120, ColumnType::Flags);
	cm->AddColumn(L"Provider", 0, 250, ColumnType::Provider);
	cm->AddColumn(L"Applicable Layer", 0, 250, ColumnType::Layer);
	cm->AddColumn(L"Provider Data", LVCFMT_RIGHT, 90, ColumnType::ProviderData);
	cm->AddColumn(L"Description", 0, 300, ColumnType::Desc);

	CImageList images;
	images.Create(16, 16, ILC_COLOR32 | ILC_MASK, 1, 1);
	images.AddIcon(AtlLoadIconImage(IDI_CALLOUT, 0, 16, 16));
	m_List.SetImageList(images, LVSIL_SMALL);

	return 0;
}

LRESULT CCalloutsView::OnRefresh(WORD, WORD, HWND, BOOL&) {
	Refresh();
	return 0;
}

void CCalloutsView::SetLayer(GUID const& layer) {
	m_LayerKey = layer;
}

void CCalloutsView::Refresh() {
	WFPCalloutEnumerator cenum(m_Engine.Handle());
	if (m_LayerKey != GUID_NULL) {
		m_Callouts = cenum.NextFiltered<CalloutInfo>([&](auto c) { return c->applicableLayer == m_LayerKey; }, 2048);
	}
	else {
		m_Callouts = cenum.Next<CalloutInfo>(2048);
	}
	Sort(m_List);
	m_List.SetItemCountEx((int)m_Callouts.size(), LVSICF_NOSCROLL);
	Frame()->SetStatusText(5, std::format(L"{} Callouts", m_Callouts.size()).c_str());
}

CString CCalloutsView::GetColumnText(HWND, int row, int col) {
	auto& info = m_Callouts[row];
	switch (GetColumnManager(m_List)->GetColumnTag<ColumnType>(col)) {
		case ColumnType::Key: return StringHelper::GuidToString(info.Data->calloutKey);
		case ColumnType::Layer: 
			if (info.Layer.IsEmpty()) {
				auto layer = m_Engine.GetLayerByKey(info.Data->applicableLayer);
				if (layer && layer->displayData.name) {
					info.Layer = layer->displayData.name;
				}
				else {
					info.Layer = StringHelper::GuidToString(info.Data->applicableLayer);
				}
			}
			return info.Layer;

		case ColumnType::Provider:
			if (info.Provider.IsEmpty()) {
				if (info.Data->providerKey) {
					auto sl = m_Engine.GetProviderByKey(*info.Data->providerKey);
					if (sl && sl->displayData.name && sl->displayData.name[0] != L'@')
						info.Provider = sl->displayData.name;
					else
						info.Provider = StringHelper::GuidToString(*info.Data->providerKey);
				}
			}
			return info.Provider;

		case ColumnType::Name: return info.Data->displayData.name;
		case ColumnType::Desc: return info.Data->displayData.description;
		case ColumnType::ProviderData:
			return info.Data->providerData.size == 0 ? L"" : std::format(L"{} Bytes", info.Data->providerData.size).c_str();

		case ColumnType::Flags:
			if (info.Data->flags == 0)
				return L"0";
			return std::format(L"0x{:X} ({})", info.Data->flags,
				(PCWSTR)StringHelper::WFPCalloutFlagsToString(info.Data->flags)).c_str();

		case ColumnType::Id: return std::to_wstring(info.Data->calloutId).c_str();
	}
	return CString();
}

void CCalloutsView::DoSort(SortInfo const* si) {
	auto col = GetColumnManager(m_List)->GetColumnTag<ColumnType>(si->SortColumn);
	auto asc = si->SortAscending;

	auto compare = [&](auto& c1, auto& c2) {
		auto d1 = c1.Data, d2 = c2.Data;
		switch (col) {
			case ColumnType::Key: return SortHelper::Sort(StringHelper::GuidToString(d1->calloutKey), StringHelper::GuidToString(d2->calloutKey), asc);
			case ColumnType::Layer: return SortHelper::Sort(c1.Layer, c2.Layer, asc);
			case ColumnType::Name: return SortHelper::Sort(d1->displayData.name, d2->displayData.name, asc);
			case ColumnType::Desc: return SortHelper::Sort(d1->displayData.description, d2->displayData.description, asc);
			case ColumnType::Flags: return SortHelper::Sort(d1->flags, d2->flags, asc);
			case ColumnType::Provider: return SortHelper::Sort(c1.Provider, c2.Provider, asc);
			case ColumnType::Id: return SortHelper::Sort(d1->calloutId, d2->calloutId, asc);
		}
		return false;
	};
	std::ranges::sort(m_Callouts, compare);
}

int CCalloutsView::GetSaveColumnRange(HWND, int&) const {
	return 1;
}

int CCalloutsView::GetRowImage(HWND, int row, int col) const {
	return 0;
}

LRESULT CCalloutsView::OnCopy(WORD, WORD, HWND, BOOL&) {
	auto text = ListViewHelper::GetSelectedRowsAsString(m_List, L",");
	ClipboardHelper::CopyText(m_hWnd, text);

	return 0;
}

LRESULT CCalloutsView::OnActivate(UINT, WPARAM active, LPARAM, BOOL&) const {
	if (active)
		UpdateUI();
	return 0;
}

void CCalloutsView::OnStateChanged(HWND, int from, int to, UINT oldState, UINT newState) const {
	if ((newState & LVIS_SELECTED) || (oldState & LVIS_SELECTED))
		UpdateUI();
}

void CCalloutsView::UpdateUI() const {
	auto& ui = Frame()->UI();
	auto selected = m_List.GetSelectedCount();
	ui.UIEnable(ID_EDIT_COPY, selected > 0);
}

bool CCalloutsView::OnRightClickList(HWND, int row, int col, POINT const& pt) const {
	if (row < 0)
		return false;

	CMenu menu;
	menu.LoadMenu(IDR_CONTEXT);

	return Frame()->TrackPopupMenu(menu.GetSubMenu(1), 0, pt.x, pt.y);
}
