#include "pch.h"
#include "WFPHelper.h"
#include <WFPEngine.h>
#include <SortHelper.h>
#include "StringHelper.h"
#include "LayerGeneralPage.h"
#include "LayerFieldsPage.h"
#include "FilterGeneralPage.h"
#include "FilterConditionsPage.h"
#include "LayersView.h"
#include "ProviderDlg.h"
#include "CalloutDlg.h"
#include "SubLayerDlg.h"
#include "WFPEnumerators.h"
#include "FiltersListPage.h"
#include "AppSettings.h"
#include <NodeGraphControl.h>
#include <WTLHelper.h>

#include <IconPropertySheet.h>
#include <ResizablePropertySheet.h>

class CPropertiesSheet : public CIconPropertySheetImpl<CPropertiesSheet, CResizablePropertySheetImpl<CPropertiesSheet>> {
public:
	using CIconPropertySheetImpl::CIconPropertySheetImpl;
};

CString WFPHelper::GetProviderName(WFPEngine const& engine, GUID const& key) {
	auto provider = engine.GetProviderByKey(key);
	if (auto name = provider ? StringHelper::ParseMUIString(provider->displayData.name) : CString(); !name.IsEmpty())
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
	if (auto name = callout ? StringHelper::ParseMUIString(callout->displayData.name) : CString(); !name.IsEmpty())
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

void WFPHelper::SaveMap(HWND hWnd, NodeGraphCtrl::CNodeGraphControl& graph, CString name) {
	// a file name, from a layer's or callout's name
	for (auto ch : { L' ', L'\\', L'/', L':', L'*', L'?', L'"', L'<', L'>', L'|' })
		name.Replace(ch, L'_');
	// the dialog gives the file the extension of the chosen type
	CSimpleFileDialog dlg(FALSE, L"png", name, OFN_EXPLORER | OFN_ENABLESIZING | OFN_OVERWRITEPROMPT,
		L"PNG Image (*.png)\0*.png\0SVG Image (*.svg)\0*.svg\0WFP Map (*.wfpmap)\0*.wfpmap\0JPEG Image (*.jpg)\0*.jpg;*.jpeg\0BMP Image (*.bmp)\0*.bmp\0", hWnd);
	WTLHelper::SuspendHook();
	auto ok = IDOK == dlg.DoModal();
	WTLHelper::ResumeHook();
	if (!ok)
		return;

	CWaitCursor wait;
	auto path = dlg.m_szFileName;
	auto ext = ::PathFindExtension(path);
	if (::_wcsicmp(ext, L".svg") == 0)
		ok = graph.GetModel()->SaveSvg(path, graph.GetBackgroundColor());
	else if (::_wcsicmp(ext, L".wfpmap") == 0)
		ok = graph.GetModel()->Save(path);
	else	// an image is drawn at 1.5 pixels per graph unit so that the text is easy to read (a very large graph gets less)
		ok = graph.SaveImage(path, 1.5f);
	if (!ok)
		AtlMessageBox(hWnd, L"Failed to save the map", IDS_TITLE, MB_ICONERROR);
}

void WFPHelper::FitGraph(NodeGraphCtrl::CNodeGraphControl& graph) {
	auto model = graph.GetModel();
	if (model == nullptr || model->Nodes().empty())
		return;
	float minX = FLT_MAX, minY = FLT_MAX, maxX = -FLT_MAX, maxY = -FLT_MAX;
	for (auto& n : model->Nodes()) {
		minX = (std::min)(minX, n.X - n.Width / 2); maxX = (std::max)(maxX, n.X + n.Width / 2);
		minY = (std::min)(minY, n.Y - n.Height / 2); maxY = (std::max)(maxY, n.Y + n.Height / 2);
	}

	graph.FitInView();
	if (graph.GetZoom() > 1.0f) {
		// a small graph stays in the middle, at its own size
		graph.CenterOn((minX + maxX) / 2, (minY + maxY) / 2, 1.0f);
	}
	else if (graph.GetZoom() < 0.7f) {
		// too much to fit: the top-left part at a readable size
		CRect rc;
		graph.GetClientRect(&rc);
		const float zoom = 0.9f;
		graph.CenterOn(minX - 20 + rc.Width() / zoom / 2, minY - 20 + rc.Height() / zoom / 2, zoom);
	}
}

UINT WFPHelper::TrackMapMenu(HWND hWnd, POINT const& pt, bool callout) {
	CMenu menu;
	menu.LoadMenu(IDR_CONTEXT);
	// the "layer map" and "callout map" popups of IDR_CONTEXT
	return (UINT)::TrackPopupMenu(menu.GetSubMenu(callout ? 5 : 3), TPM_RETURNCMD | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, nullptr);
}

int WFPHelper::ShowLayerProperties(WFPEngine& engine, FWPM_LAYER* layer) {
	auto name = L"Layer Properties (" + GetLayerName(engine, layer->layerKey) + L")";
	CPropertiesSheet sheet((PCWSTR)name);
	sheet.m_psh.dwFlags |= PSH_NOAPPLYNOW | PSH_USEICONID | PSH_NOCONTEXTHELP;
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
	CFiltersListPage filterPage(engine, layer);
	filterPage.SetTitle(L"Filters");
	filterPage.m_psp.dwFlags |= PSP_USEICONID;
	filterPage.m_psp.pszIcon = MAKEINTRESOURCE(IDI_FILTER);
	sheet.AddPage(filterPage);
	// the size the user gave the sheet last time
	auto& settings = AppSettings::Get();
	sheet.SetSize(settings.LayerPropertiesSize());
	auto result = (int)sheet.DoModal();
	settings.LayerPropertiesSize(sheet.GetSize());
	return result;
}

int WFPHelper::ShowFilterProperties(WFPEngine& engine, FWPM_FILTER* filter) {
	auto name = L"Filter: " + GetFilterName(engine, filter->filterKey);
	CPropertiesSheet sheet((PCWSTR)name);
	sheet.m_psh.dwFlags |= PSH_NOAPPLYNOW | PSH_USEICONID | PSH_NOCONTEXTHELP;
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
	auto& settings = AppSettings::Get();
	sheet.SetSize(settings.FilterPropertiesSize());
	auto result = (int)sheet.DoModal();
	settings.FilterPropertiesSize(sheet.GetSize());
	return result;
}

int WFPHelper::ShowSublayerProperties(WFPEngine& engine, FWPM_SUBLAYER* sublayer) {
	CSubLayerDlg dlg(engine, sublayer);
	dlg.DoModal();

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
		case FWP_UINT8 + (FWP_UINT8 << 8) : return SortHelper::Sort(v1.uint8, v2.uint8, asc);
			case FWP_UINT8 + (FWP_UINT64 << 8) : return SortHelper::Sort<UINT64>(v1.uint8, *v2.uint64, asc);
				case FWP_UINT64 + (FWP_UINT8 << 8) : return SortHelper::Sort<UINT64>(*v1.uint64, v2.uint8, asc);
					case FWP_UINT16 + (FWP_UINT16 << 8) : return SortHelper::Sort(v1.uint16, v2.uint16, asc);
						case FWP_UINT32 + (FWP_UINT32 << 8) : return SortHelper::Sort(v1.uint32, v2.uint32, asc);
							case FWP_UINT64 + (FWP_UINT64 << 8) : return SortHelper::Sort(*v1.uint64, *v2.uint64, asc);
								case FWP_INT8 + (FWP_INT64 << 8) : return SortHelper::Sort(v1.int8, *v2.int64, asc);
									case FWP_INT16 + (FWP_INT16 << 8) : return SortHelper::Sort(v1.int16, v2.int16, asc);
										case FWP_INT32 + (FWP_INT32 << 8) : return SortHelper::Sort(v1.int32, v2.int32, asc);
											case FWP_INT64 + (FWP_INT64 << 8) : return SortHelper::Sort(*v1.int64, *v2.int64, asc);
	}
	return false;
}
