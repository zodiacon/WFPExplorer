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
		case 1: return StringHelper::WFPFieldTypeToString(field.type);
		case 2: return StringHelper::WFPDataTypeToString((WFPDataType)field.dataType);
	}
	return CString();
}

void CLayerFieldsPage::DoSort(SortInfo const* si) {
	auto compare = [&](auto& f1, auto& f2) {
		switch (si->SortColumn) {
			case 0: return SortHelper::Sort(StringHelper::WFPConditionFieldKeyToString(*f1.fieldKey), StringHelper::WFPConditionFieldKeyToString(*f2.fieldKey), si->SortAscending);
			case 1: return SortHelper::Sort(f1.type, f2.type, si->SortAscending);
			case 2: return SortHelper::Sort(StringHelper::WFPDataTypeToString((WFPDataType)f1.dataType), StringHelper::WFPDataTypeToString((WFPDataType)f2.dataType), si->SortAscending);
		}
		return false;
	};
	std::ranges::sort(m_Fields, compare);
}

LRESULT CLayerFieldsPage::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
	InitDynamicLayout(false);
	m_List.Attach(GetDlgItem(IDC_LIST));
	m_List.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT);

	auto layer = m_Engine.GetLayerByKey(m_Layer->layerKey);
	m_Fields.resize(layer->numFields);
	memcpy(m_Fields.data(), layer->field, sizeof(FWPM_FIELD) * layer->numFields);
	m_List.SetItemCount(m_Layer->numFields);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"Field", 0, 230);
	cm->AddColumn(L"Type", 0, 80);
	cm->AddColumn(L"Data Type", 0, 150);

	return 0;
}
