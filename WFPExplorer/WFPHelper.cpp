#include "pch.h"
#include "WFPHelper.h"
#include <WFPEngine.h>
#include "StringHelper.h"
#include "LayerGeneralPage.h"
#include "LayerFieldsPage.h"
#include "FilterGeneralPage.h"
#include "FilterConditionsPage.h"
#include "LayersView.h"
#include "ProviderDlg.h"
#include "CalloutDlg.h"

CString WFPHelper::GetProviderName(WFPEngine const& engine, GUID const& key) {
	auto provider = engine.GetProviderByKey(key);
	if (auto name = StringHelper::ParseMUIString(provider->displayData.name); !name.IsEmpty())
		return name;
	return StringHelper::GuidToString(key);
}

CString WFPHelper::GetFilterName(WFPEngine const& engine, GUID const& key) {
	if (auto filter = engine.GetFilterByKey(key); filter)
		return StringHelper::ParseMUIString(filter->displayData.name);
	return StringHelper::GuidToString(key);
}

CString WFPHelper::GetLayerName(WFPEngine const& engine, GUID const& key) {
	if (key != GUID_NULL) {
		auto layer = engine.GetLayerByKey(key);
		if (layer && layer->displayData.name && layer->displayData.name[0] != L'@')
			return layer->displayData.name;
		return StringHelper::GuidToString(key);
	}
	return L"";
}

CString WFPHelper::GetCalloutName(WFPEngine const& engine, GUID const& key) {
	auto callout = engine.GetCalloutByKey(key);
	if (auto name = StringHelper::ParseMUIString(callout->displayData.name); !name.IsEmpty())
		return name;
	return StringHelper::GuidToString(key);
}

CString WFPHelper::GetSublayerName(WFPEngine const& engine, GUID const& key) {
	if (key != GUID_NULL) {
		auto layer = engine.GetSublayerByKey(key);
		if (layer)
			return StringHelper::ParseMUIString(layer->displayData.name);
		return StringHelper::GuidToString(key);
	}
	return L"";
}

int WFPHelper::ShowLayerProperties(WFPEngine& engine, FWPM_LAYER* layer) {
	auto name = L"Layer Properties (" + GetLayerName(engine, layer->layerKey) + L")";
	CPropertySheet sheet((PCWSTR)name);
	sheet.m_psh.dwFlags |= PSH_NOAPPLYNOW | PSH_USEICONID | PSH_NOCONTEXTHELP | PSH_RESIZABLE;
	sheet.m_psh.pszIcon = MAKEINTRESOURCE(IDI_LAYERS);
	CLayerGeneralPage general(engine, layer);
	general.m_psp.dwFlags |= PSP_USEICONID;
	general.m_psp.pszIcon = MAKEINTRESOURCE(IDI_CUBE);
	sheet.AddPage(general);

	CLayerFieldsPage fields(engine, layer);
	if (layer->numFields > 0) {
		fields.m_psp.dwFlags |= PSP_USEICONID;
		fields.m_psp.pszIcon = MAKEINTRESOURCE(IDI_FIELD);
		sheet.AddPage(fields);
	}
	return (int)sheet.DoModal();
}

int WFPHelper::ShowFilterProperties(WFPEngine& engine, FWPM_FILTER* filter) {
	auto name = L"Filter: " + GetFilterName(engine, filter->filterKey);
	CPropertySheet sheet((PCWSTR)name);
	sheet.m_psh.dwFlags |= PSH_NOAPPLYNOW | PSH_USEICONID | PSH_NOCONTEXTHELP | PSH_RESIZABLE;
	sheet.m_psh.pszIcon = MAKEINTRESOURCE(IDI_FILTER);
	CFilterGeneralPage general(engine, filter);
	general.m_psp.dwFlags |= PSP_USEICONID;
	general.m_psp.pszIcon = MAKEINTRESOURCE(IDI_CUBE);
	CFilterConditionsPage cond(engine, filter);
	sheet.AddPage(general);
	if (filter->numFilterConditions > 0) {
		cond.m_psp.dwFlags |= PSP_USEICONID;
		cond.m_psp.pszIcon = MAKEINTRESOURCE(IDI_CONDITION);
		sheet.AddPage(cond);
	}
	return (int)sheet.DoModal();
}

int WFPHelper::ShowSublayerProperties(WFPEngine& engine, FWPM_SUBLAYER* sublayer) {
	return 0;
}

int WFPHelper::ShowProviderProperties(WFPEngine& engine, FWPM_PROVIDER* provider) {
	CProviderDlg dlg(provider);
	return (int)dlg.DoModal();
}

int WFPHelper::ShowCalloutProperties(WFPEngine& engine, FWPM_CALLOUT* callout) {
	CCalloutDlg dlg(engine, callout);
	return (int)dlg.DoModal();
}
