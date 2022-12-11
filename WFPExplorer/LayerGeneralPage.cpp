#include "pch.h"
#include "LayerGeneralPage.h"
#include <WFPEngine.h>
#include "StringHelper.h"
#include "WFPHelper.h"

CLayerGeneralPage::CLayerGeneralPage(WFPEngine& engine, FWPM_LAYER* layer) : m_Engine(engine), m_Layer(layer) {
}

LRESULT CLayerGeneralPage::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
	SetDlgItemText(IDC_NAME, StringHelper::ParseMUIString(m_Layer->displayData.name));
	SetDlgItemText(IDC_DESC, StringHelper::ParseMUIString(m_Layer->displayData.description));
	SetDlgItemText(IDC_KEY, StringHelper::GuidToString(m_Layer->layerKey));
	SetDlgItemInt(IDC_FIELDS, m_Layer->numFields);
	SetDlgItemInt(IDC_ID, m_Layer->layerId);
	auto flags = StringHelper::WFPLayerFlagsToString(m_Layer->flags);
	if (!flags.IsEmpty())
		flags = std::format(L"0x{:X} ({})", m_Layer->flags, (PCWSTR)flags).c_str();
	else
		flags = L"(None)";
	SetDlgItemText(IDC_FLAGS, flags);
	SetDlgItemText(IDC_SUBLAYER, WFPHelper::GetSublayerName(m_Engine, m_Layer->defaultSubLayerKey));
	SetDlgItemInt(IDC_FILTERS, m_Engine.GetFilterCount(m_Layer->layerKey), false);
	SetDlgItemInt(IDC_CALLOUTS, m_Engine.GetCalloutCount(m_Layer->layerKey), false);

	AddIconToButton(IDC_SUBLAYER_PROP, IDI_SUBLAYER);

	return 0;
}
