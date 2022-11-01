#include "pch.h"
#include "FiltersView.h"
#include <WFPEngine.h>
#include "StringHelper.h"

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
		case ColumnType::Flags: return std::to_wstring(info.Flags).c_str();
		case ColumnType::EffectiveWeight: return StringHelper::WFPValueToString(info.EffectiveWeight, true);
		case ColumnType::ProviderName:
			if (info.ProviderName.IsEmpty() && info.ProviderKey != GUID_NULL) {
				auto provider = m_Engine.GetProviderByKey(info.ProviderKey);
				if (provider)
					info.ProviderName = provider.Name.c_str();
			}
			return info.ProviderName;

	}
	return CString();
}

LRESULT CFiltersView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	m_hWndClient = m_List.Create(m_hWnd, rcDefault, nullptr,
		WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | LVS_OWNERDATA | LVS_REPORT | LVS_SHOWSELALWAYS);
	m_List.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"Filter Key", 0, 250, ColumnType::Key);
	cm->AddColumn(L"Weight", LVCFMT_RIGHT, 90, ColumnType::Weight);
	cm->AddColumn(L"Effective Weight", LVCFMT_RIGHT, 90, ColumnType::EffectiveWeight);
	cm->AddColumn(L"Flags", LVCFMT_RIGHT, 80, ColumnType::Flags);
	cm->AddColumn(L"Filter Name", 0, 180, ColumnType::Name);
	cm->AddColumn(L"Description", 0, 180, ColumnType::Desc);
	cm->AddColumn(L"Provider Name", 0, 180, ColumnType::ProviderName);
	cm->AddColumn(L"Layer Key", LVCFMT_LEFT, 250, ColumnType::LayerKey);
	cm->AddColumn(L"Sublayer Key", LVCFMT_LEFT, 250, ColumnType::SubLayerKey);

	Refresh();

	return 0;
}

LRESULT CFiltersView::OnRefresh(WORD, WORD, HWND, BOOL&) {
	Refresh();
	return 0;
}
