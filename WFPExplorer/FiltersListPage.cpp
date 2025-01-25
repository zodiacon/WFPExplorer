#include "pch.h"
#include "FiltersListPage.h"
#include <WFPEngine.h>
#include "StringHelper.h"
#include <SortHelper.h>

CFiltersListPage::CFiltersListPage(WFPEngine& engine, FWPM_LAYER* layer) : m_Engine(engine), m_Layer(layer) {
	WFPFilterEnumerator enumerator(engine.Handle());
	DWORD count = 0;
	DWORD maxCount = 8000;
	do {
		m_Filters.Append(enumerator.NextFiltered<FilterInfo>([&](auto& filter) { return filter->layerKey == layer->layerKey; }, maxCount));
		count += maxCount;
	} while (m_Filters.size() == count);

}

size_t CFiltersListPage::GetFilterCount() const {
	return m_Filters.size();
}

CString CFiltersListPage::GetColumnText(HWND, int row, int col) const {
	auto& f = m_Filters[row];
	switch (col) {
		case 0: return std::to_wstring(f.Index).c_str();
		case 1: return f.Data->displayData.name;
		case 2: return std::to_wstring(f.Data->numFilterConditions).c_str();
		case 3: return std::format(L"0x{:X}", *f.Data->effectiveWeight.uint64).c_str();
		case 4: return StringHelper::WFPFilterActionTypeToString(f.Data->action.type);
	}
	return CString();
}

void CFiltersListPage::DoSort(SortInfo const* si) {
	auto sort = [&](auto& f1, auto& f2) {
		switch (si->SortColumn) {
			case 0: return SortHelper::Sort(f1.Index, f2.Index, si->SortAscending);
			case 1: return SortHelper::Sort(f1.Data->displayData.name, f2.Data->displayData.name, si->SortAscending);
			case 2: return SortHelper::Sort(f1.Data->numFilterConditions, f2.Data->numFilterConditions, si->SortAscending);
			case 3: return SortHelper::Sort(*f1.Data->effectiveWeight.uint64, *f2.Data->effectiveWeight.uint64, si->SortAscending);
			case 4: return SortHelper::Sort(StringHelper::WFPFilterActionTypeToString(f1.Data->action.type), StringHelper::WFPFilterActionTypeToString(f2.Data->action.type), si->SortAscending);
		}
		return false;
		};

	std::ranges::sort(m_Filters, sort);
}

int CFiltersListPage::GetRowImage(HWND, int row, int) const {
	switch (m_Filters[row].Data->action.type) {
		case FWP_ACTION_BLOCK: return 2;
		case FWP_ACTION_PERMIT: return 1;
		case FWP_ACTION_CONTINUE: return 3;
	}
	return 4;
}

LRESULT CFiltersListPage::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
	InitDynamicLayout(false);
	m_List.Attach(GetDlgItem(IDC_LIST));
	m_List.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT);

	CImageList images;
	images.Create(16, 16, ILC_COLOR32 | ILC_MASK, 6, 4);
	UINT icons[] = { IDI_FILTER, IDI_FILTER_PERMIT, IDI_FILTER_BLOCK, IDI_FILTER_REFRESH, IDI_CALLOUT };
	for (auto icon : icons)
		images.AddIcon(AtlLoadIconImage(icon, 0, 16, 16));
	m_List.SetImageList(images, LVSIL_SMALL);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"", 0, 0);
	cm->AddColumn(L"#", LVCFMT_RIGHT, 50);
	cm->AddColumn(L"Name", 0, 240);
	cm->AddColumn(L"Conditions", LVCFMT_RIGHT, 60);
	cm->AddColumn(L"Weight", LVCFMT_RIGHT, 130);
	cm->AddColumn(L"Action", 0, 110);
	cm->DeleteColumn(0);

	int i = 1;
	for (auto& f : m_Filters)
		f.Index = i++;

	m_List.SetItemCount((int)m_Filters.size());
	return 0;
}
