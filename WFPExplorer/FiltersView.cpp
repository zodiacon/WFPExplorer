#include "pch.h"
#include "FiltersView.h"
#include <WFPEngine.h>
#include "StringHelper.h"
#include <SortHelper.h>
#include "resource.h"

CFiltersView::CFiltersView(IMainFrame* frame, WFPEngine& engine) : CFrameView(frame), m_Engine(engine) {
}

void CFiltersView::Refresh() {
	CWaitCursor wait;
	m_Filters = m_Engine.EnumFilters<FilterInfo>();
	m_List.SetItemCountEx((int)m_Filters.size(), LVSICF_NOSCROLL);
}

CString CFiltersView::GetColumnText(HWND, int row, int col) {
	auto& info = m_Filters[row];
	switch (GetColumnManager(m_List)->GetColumnTag<ColumnType>(col)) {
		case ColumnType::Key: return StringHelper::GuidToString(info.FilterKey);
		case ColumnType::Name: return info.Name.c_str();
		case ColumnType::Desc: return info.Desc.c_str();
		case ColumnType::Id: return std::to_wstring(info.FilterId).c_str();
		case ColumnType::LayerKey: return StringHelper::GuidToString(info.LayerKey);
		case ColumnType::SubLayerKey: return StringHelper::GuidToString(info.SubLayerKey);
		case ColumnType::Weight: return StringHelper::WFPValueToString(info.Weight, true);
		case ColumnType::Flags: 
			if (info.Flags == WFPFilterFlags::None)
				return L"0";
			return std::format(L"0x{:X} ({})", (UINT32)info.Flags, 
				(PCWSTR)StringHelper::WFPFilterFlagsToString(info.Flags)).c_str();
		case ColumnType::EffectiveWeight: return StringHelper::WFPValueToString(info.EffectiveWeight, true);
		case ColumnType::ProviderName: return GetProviderName(info);
		case ColumnType::Layer: return GetLayerName(info);
		case ColumnType::SubLayer: return GetSubLayerName(info);
	}
	return CString();
}

CString const& CFiltersView::GetProviderName(FilterInfo& info) const {
	if (info.ProviderName.IsEmpty() && info.ProviderKey != GUID_NULL) {
		auto provider = m_Engine.GetProviderByKey(info.ProviderKey);
		if (provider && !provider.value().Name.empty() && provider.value().Name[0] != L'@')
			info.ProviderName = provider.value().Name.c_str();
		else
			info.ProviderName = StringHelper::GuidToString(info.ProviderKey);
	}
	return info.ProviderName;
}

CString const& CFiltersView::GetLayerName(FilterInfo& info) const {
	if (info.Layer.IsEmpty() && info.LayerKey != GUID_NULL) {
		auto layer = m_Engine.GetLayerByKey(info.LayerKey);
		if (layer && !layer.value().Name.empty() && layer.value().Name[0] != L'@')
			info.Layer = layer.value().Name.c_str();
		else
			info.Layer = StringHelper::GuidToString(info.LayerKey);
	}
	return info.Layer;
}

CString const& CFiltersView::GetSubLayerName(FilterInfo& info) const {
	if (info.SubLayer.IsEmpty() && info.SubLayerKey != GUID_NULL) {
		auto layer = m_Engine.GetSubLayerByKey(info.SubLayerKey);
		if (layer && !layer.value().Name.empty() && layer.value().Name[0] != L'@')
			info.SubLayer = layer.value().Name.c_str();
		else
			info.SubLayer = StringHelper::GuidToString(info.LayerKey);
	}
	return info.SubLayer;
}

LRESULT CFiltersView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	m_hWndClient = m_List.Create(m_hWnd, rcDefault, nullptr,
		WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | LVS_OWNERDATA | LVS_REPORT | LVS_SHOWSELALWAYS);
	m_List.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"Filter Key", 0, 250, ColumnType::Key);
	cm->AddColumn(L"Weight", LVCFMT_RIGHT, 90, ColumnType::Weight);
	cm->AddColumn(L"Effective Weight", LVCFMT_RIGHT, 90, ColumnType::EffectiveWeight);
	cm->AddColumn(L"Flags", LVCFMT_LEFT, 150, ColumnType::Flags);
	cm->AddColumn(L"Filter Name", 0, 180, ColumnType::Name);
	cm->AddColumn(L"Description", 0, 180, ColumnType::Desc);
	cm->AddColumn(L"Provider", 0, 240, ColumnType::ProviderName);
	cm->AddColumn(L"Layer", LVCFMT_LEFT, 250, ColumnType::Layer);
	cm->AddColumn(L"Sublayer", LVCFMT_LEFT, 250, ColumnType::SubLayer);

	CImageList images;
	images.Create(16, 16, ILC_COLOR32 | ILC_MASK, 1, 1);
	images.AddIcon(AtlLoadIconImage(IDI_FILTER, 0, 16, 16));
	m_List.SetImageList(images, LVSIL_SMALL);

	Refresh();

	return 0;
}

LRESULT CFiltersView::OnRefresh(WORD, WORD, HWND, BOOL&) {
	Refresh();
	return 0;
}

void CFiltersView::DoSort(SortInfo const* si) {
	auto col = GetColumnManager(m_List)->GetColumnTag<ColumnType>(si->SortColumn);
	auto asc = si->SortAscending;

	auto compare = [&](auto& f1, auto& f2) {
		switch (col) {
			case ColumnType::Key: return SortHelper::Sort(StringHelper::GuidToString(f1.FilterKey), StringHelper::GuidToString(f2.FilterKey), asc);
			case ColumnType::Name: return SortHelper::Sort(f1.Name, f2.Name, asc);
			case ColumnType::Desc: return SortHelper::Sort(f1.Desc, f2.Desc, asc);
			case ColumnType::Flags: return SortHelper::Sort(f1.Flags, f2.Flags, asc);
			case ColumnType::ProviderName: return SortHelper::Sort(GetProviderName(f1), GetProviderName(f2), asc);
			case ColumnType::Weight: return SortHelper::Sort(f1.Weight.uint8, f2.Weight.uint8, asc);
			case ColumnType::EffectiveWeight: return SortHelper::Sort(f1.EffectiveWeight.uint64, f2.EffectiveWeight.uint64, asc);
			case ColumnType::Layer: return SortHelper::Sort(GetLayerName(f1), GetLayerName(f2), asc);
			case ColumnType::SubLayer: return SortHelper::Sort(GetSubLayerName(f1), GetSubLayerName(f2), asc);
		}
		return false;
	};
	std::ranges::sort(m_Filters, compare);
}

int CFiltersView::GetSaveColumnRange(HWND, int&) const {
	return 1;
}

int CFiltersView::GetRowImage(HWND, int row, int col) const {
	return 0;
}
