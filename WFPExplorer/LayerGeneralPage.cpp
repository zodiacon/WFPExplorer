#include "pch.h"
#include "LayerGeneralPage.h"
#include <WFPEngine.h>
#include "StringHelper.h"
#include "WFPHelper.h"

CLayerGeneralPage::CLayerGeneralPage(WFPEngine& engine, WFPLayerInfo const& layer) : m_Engine(engine), m_Layer(layer) {
}

LRESULT CLayerGeneralPage::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
	SetDlgItemText(IDC_NAME, m_Layer.Name.c_str());
	SetDlgItemText(IDC_DESC, m_Layer.Desc.c_str());
	SetDlgItemText(IDC_KEY, StringHelper::GuidToString(m_Layer.LayerKey));
	SetDlgItemInt(IDC_FIELDS, m_Layer.NumFields);
	SetDlgItemInt(IDC_ID, m_Layer.LayerId);
	auto flags = StringHelper::WFPLayerFlagsToString(m_Layer.Flags);
	if (!flags.IsEmpty())
		flags = std::format(L"0x{:X} ({})", (UINT32)m_Layer.Flags, (PCWSTR)flags).c_str();
	else
		flags = L"(None)";
	SetDlgItemText(IDC_FLAGS, flags);
	SetDlgItemText(IDC_SUBLAYER, WFPHelper::GetSublayerName(m_Engine, m_Layer.DefaultSubLayerKey));
	SetDlgItemInt(IDC_FILTERS, m_Engine.GetFilterCount(m_Layer.LayerKey), false);
	SetDlgItemInt(IDC_CALLOUTS, m_Engine.GetCalloutCount(m_Layer.LayerKey), false);

	AddIconToButton(IDC_SUBLAYER_PROP, IDI_SUBLAYER);

	return 0;
}
