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
#include <SortHelper.h>

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

bool WFPHelper::Sort(FWP_VALUE const& v1, FWP_VALUE const& v2, bool asc) {
	if (v1.type != v2.type) {
		if (v1.type == FWP_EMPTY)
			return asc;
		if (v2.type == FWP_EMPTY)
			return !asc;
	}

	//
	// cover some common cases...
	//
	switch (v1.type + (v2.type << 8)) {
		case FWP_UINT8 + (FWP_UINT8 << 8): return SortHelper::Sort(v1.uint8, v2.uint8, asc);
		case FWP_UINT8 + (FWP_UINT64 << 8) : return SortHelper::Sort<UINT64>(v1.uint8, *v2.uint64, asc);
		case FWP_UINT64 + (FWP_UINT8 << 8) : return SortHelper::Sort<UINT64>(*v1.uint64, v2.uint8, asc);
		case FWP_UINT16 + (FWP_UINT16 << 8) : return SortHelper::Sort(v1.uint16, v2.uint16, asc);
		case FWP_UINT32 + (FWP_UINT32 << 8) : return SortHelper::Sort(v1.uint32, v2.uint32, asc);
		case FWP_UINT64 + (FWP_UINT64 << 8) : return SortHelper::Sort(*v1.uint64, *v2.uint64, asc);
		case FWP_INT8 + (FWP_INT64 << 8) : return SortHelper::Sort(v1.int8, *v2.int64, asc);
		case FWP_INT16 + (FWP_INT16 << 8) : return SortHelper::Sort(v1.int16, v2.int16, asc);
		case FWP_INT32 + (FWP_INT32 << 8): return SortHelper::Sort(v1.int32, v2.int32, asc);
		case FWP_INT64 + (FWP_INT64 << 8): return SortHelper::Sort(*v1.int64, *v2.int64, asc);
	}
	return false;
}
