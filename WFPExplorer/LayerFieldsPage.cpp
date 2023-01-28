#include "pch.h"
#include "LayerFieldsPage.h"
#include <WFPEngine.h>
#include "StringHelper.h"
#include <SortHelper.h>

CLayerFieldsPage::CLayerFieldsPage(WFPEngine& engine, FWPM_LAYER* layer) : m_Engine(engine), m_Layer(layer) {
}

CString CLayerFieldsPage::GetColumnText(HWND, int row, int col) const {
	auto const& field = m_Fields[row];

	switch (col) {
		case 0: return StringHelper::WFPConditionFieldKeyToString(*field.fieldKey);
		case 1: return std::to_wstring(field.Index).c_str();
		case 2: return StringHelper::WFPFieldTypeToString(field.type);
		case 3: return StringHelper::WFPDataTypeToString(field.dataType);
	}
	return CString();
}

void CLayerFieldsPage::DoSort(SortInfo const* si) {
	auto compare = [&](auto& f1, auto& f2) {
		switch (si->SortColumn) {
			case 0: return SortHelper::Sort(StringHelper::WFPConditionFieldKeyToString(*f1.fieldKey), StringHelper::WFPConditionFieldKeyToString(*f2.fieldKey), si->SortAscending);
			case 1: return SortHelper::Sort(f1.Index, f2.Index, si->SortAscending);
			case 2: return SortHelper::Sort(f1.type, f2.type, si->SortAscending);
			case 3: return SortHelper::Sort(StringHelper::WFPDataTypeToString(f1.dataType), StringHelper::WFPDataTypeToString(f2.dataType), si->SortAscending);
		}
		return false;
	};
	std::ranges::sort(m_Fields, compare);
}

int CLayerFieldsPage::GetRowImage(HWND, int row, int) const {
	return 0;
}

LRESULT CLayerFieldsPage::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
	InitDynamicLayout(false);
	m_List.Attach(GetDlgItem(IDC_LIST));
	m_List.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT);

	auto count = m_Layer->numFields;
	m_Fields.reserve(count);
	for (UINT32 i = 0; i < count; i++) {
		FieldInfo info(m_Layer->field[i]);
		info.Index = i;
		m_Fields.push_back(info);
	}
	m_List.SetItemCount(count);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"Field", 0, 230);
	cm->AddColumn(L"#", LVCFMT_RIGHT, 50);
	cm->AddColumn(L"Type", 0, 80);
	cm->AddColumn(L"Data Type", 0, 150);

	CImageList images;
	images.Create(16, 16, ILC_COLOR32 | ILC_MASK, 2, 2);
	images.AddIcon(AtlLoadIconImage(IDI_FIELD, 0, 16, 16));
	m_List.SetImageList(images, LVSIL_SMALL);

	return 0;
}
