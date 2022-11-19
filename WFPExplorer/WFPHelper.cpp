#include "pch.h"
#include "WFPHelper.h"
#include <WFPEngine.h>
#include "StringHelper.h"
#include "LayerGeneralPage.h"
#include "FilterGeneralPage.h"
#include "FilterConditionsPage.h"

CString WFPHelper::GetProviderName(WFPEngine const& engine, GUID const& key) {
	if (key != GUID_NULL) {
		auto provider = engine.GetProviderByKey(key);
		if (provider && !provider->Name.empty() && provider->Name[0] != L'@')
			return provider->Name.c_str();
		return StringHelper::GuidToString(key);
	}
	return L"";
}

CString WFPHelper::GetFilterName(WFPEngine const& engine, GUID const& key) {
	if (key != GUID_NULL) {
		auto filter = engine.GetFilterByKey(key);
		if (filter && !filter->Name.empty() && filter->Name[0] != L'@')
			return filter->Name.c_str();
		return StringHelper::GuidToString(key);
	}
	return L"";
}

CString WFPHelper::GetLayerName(WFPEngine const& engine, GUID const& key) {
	if (key != GUID_NULL) {
		auto layer = engine.GetLayerByKey(key);
		if (layer && !layer->Name.empty() && layer->Name[0] != L'@')
			return layer->Name.c_str();
		return StringHelper::GuidToString(key);
	}
	return L"";
}

CString WFPHelper::GetSublayerName(WFPEngine const& engine, GUID const& key) {
	if (key != GUID_NULL) {
		auto layer = engine.GetSublayerByKey(key);
		if (layer && !layer->Name.empty() && layer->Name[0] != L'@')
			return layer->Name.c_str();
		return StringHelper::GuidToString(key);
	}
	return L"";
}

int WFPHelper::ShowLayerProperties(WFPEngine& engine, WFPLayerInfo& layer) {
	auto name = L"Layer Properties (" + GetLayerName(engine, layer.LayerKey) + L")";
	CPropertySheet sheet((PCWSTR)name);
	sheet.m_psh.dwFlags |= PSH_NOAPPLYNOW | PSH_USEICONID | PSH_NOCONTEXTHELP | PSH_RESIZABLE;
	sheet.m_psh.pszIcon = MAKEINTRESOURCE(IDI_LAYERS);
	CLayerGeneralPage general(engine, layer);
	general.m_psp.dwFlags |= PSP_USEICONID;
	general.m_psp.pszIcon = MAKEINTRESOURCE(IDI_CUBE);
	sheet.AddPage(general);

	return (int)sheet.DoModal();
}

int WFPHelper::ShowFilterProperties(WFPEngine& engine, WFPFilterInfo& filter) {
	auto name = L"Filter Properties (" + GetFilterName(engine, filter.FilterKey) + L")";
	CPropertySheet sheet((PCWSTR)name);
	sheet.m_psh.dwFlags |= PSH_NOAPPLYNOW | PSH_USEICONID | PSH_NOCONTEXTHELP | PSH_RESIZABLE;
	sheet.m_psh.pszIcon = MAKEINTRESOURCE(IDI_FILTER);
	CFilterGeneralPage general(engine, filter);
	general.m_psp.dwFlags |= PSP_USEICONID;
	general.m_psp.pszIcon = MAKEINTRESOURCE(IDI_CUBE);
	CFilterConditionsPage cond(engine, filter);
	sheet.AddPage(general);
	if (filter.ConditionCount > 0) {
		cond.m_psp.dwFlags |= PSP_USEICONID;
		cond.m_psp.pszIcon = MAKEINTRESOURCE(IDI_CONDITION);
		sheet.AddPage(cond);
	}
	return (int)sheet.DoModal();
}
