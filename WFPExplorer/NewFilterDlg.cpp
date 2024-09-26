#include "pch.h"
#include "resource.h"
#include "NewFilterDlg.h"
#include <WFPEngine.h>
#include <WFPEnumerators.h>
#include "StringHelper.h"
#include "WFPHelper.h"

using namespace std::ranges;
using namespace std::views;
using namespace std;

CNewFilterDlg::CNewFilterDlg(FWPM_FILTER* filter) : m_Filter(filter) {
	ATLASSERT(filter);
}

bool CNewFilterDlg::IsNewFilter() const {
	return m_Filter->filterKey == GUID_NULL;
}

LRESULT CNewFilterDlg::ReportOpenEngineFailure(PCWSTR errorText) const {
	AtlMessageBox(m_hWnd, errorText ? errorText : L"Failed to open WFP engine.", L"New Filter", MB_ICONERROR);
	return 0;
}

LRESULT CNewFilterDlg::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
	SetDialogIcon(IDI_FILTER);
	UINT ids[] = { IDC_PROVIDER_PROP, IDC_LAYER_PROP, IDC_SUBLAYER_PROP };
	for (auto id : ids) {
		CButton button(GetDlgItem(id));
		button.SetIcon(AtlLoadIconImage(IDI_INFO, 0, 16, 16));
	}

	struct {
		PCWSTR text;
		FWP_ACTION_TYPE type;
	} actions[] = {
		{ L"Block", FWP_ACTION_BLOCK },
		{ L"Permit", FWP_ACTION_PERMIT },
		{ L"Callout Terminating", FWP_ACTION_CALLOUT_TERMINATING },
		{ L"Callout Inspection", FWP_ACTION_CALLOUT_INSPECTION },
		{ L"Callout Unknown", FWP_ACTION_CALLOUT_UNKNOWN },
	};
	InitCombo(IDC_ACTION, actions, m_Filter->action.type);

	WFPEngine engine;
	if (engine.Open()) {
		{
			//
			// add providers to combo
			//
			WFPProviderEnumerator en(engine.Handle());
			auto providers = en.Next(256);
			struct Data {
				CString text;
				int type;
			};
			int selected = -1, i = 0;
			auto data = providers | std::views::transform([&](auto& p) {
				if (selected < 0 && m_Filter->providerKey && IsNewFilter() && p->providerKey == *m_Filter->providerKey)
					selected = i;
				i++;

				return Data{ p->displayData.name + CString(" (") + StringHelper::GuidToString(p->providerKey) + L")", i - 1 };
				}) | to<std::vector>();

			data.insert(data.begin(), { L"(None)", -1 });
			InitCombo(IDC_PROVIDER, data.data(), (int)data.size(), selected);
		}
		{
			//
			// add layers to combo
			//
			WFPLayerEnumerator en(engine.Handle());
			auto layers = en.Next(256);
			struct Data {
				CString text;
				int type;
			};
			auto data = layers | views::transform([&](auto& l) {
				return Data{ WFPHelper::GetLayerName(engine, l->layerKey), (int)l->layerId };
				}) | to<vector>();

			int layerId = 0;
			if (!IsNewFilter())
				layerId = engine.GetLayerByKey(m_Filter->layerKey)->layerId;
			InitCombo(IDC_LAYER, data.data(), (int)data.size(), layerId);
		}
		{
			//
			// add sublayers to combo
			//
			WFPSubLayerEnumerator en(engine.Handle());
			auto layers = en.Next(256);
			struct Data {
				CString text;
				int type;
			};
			int selected = -1, i = 0;
			auto data = layers | views::transform([&](auto& l) {
				if (selected < 0 && IsNewFilter() && l->subLayerKey == m_Filter->subLayerKey)
					selected = i;
				i++;
				return Data{ WFPHelper::GetSublayerName(engine, l->subLayerKey) + CString(L" (") + StringHelper::GuidToString(l->subLayerKey) + L")", i - 1 };
				}) | to<vector>();

			InitCombo(IDC_SUBLAYER, data.data(), (int)data.size(), selected);
		}
		{
			//
			// add callouts to combo
			//
			WFPCalloutEnumerator en(engine.Handle());
			auto callouts = en.Next(512);
			struct Data {
				CString text;
				UINT32 type;
			};
			auto data = callouts | std::views::transform([&](auto& c) {
				return Data{ 
					c->displayData.name + CString(L" (") + engine.GetLayerByKey(c->applicableLayer)->displayData.name + L")", 
					c->calloutId 
					};
				}) | to<std::vector>();

			UINT32 id = 0;
			if (!IsNewFilter())
				id = engine.GetCalloutByKey(m_Filter->action.calloutKey)->calloutId;
			InitCombo(IDC_CALLOUT, data.data(), (int)data.size(), id);
		}

		auto cb = (CComboBox)GetDlgItem(IDC_WEIGHTRANGE);
		for (int i = 0; i < 16; i++) {
			int n = cb.AddString(std::format(L"0x{:X}", i).c_str());
			cb.SetItemData(n, i);
		}
		cb.SetCurSel(0);

		if (IsNewFilter()) {
			CheckDlgButton(IDC_AUTOMATIC, BST_CHECKED);
		}
		else if (m_Filter->weight.type == FWP_EMPTY) {
			CheckDlgButton(IDC_AUTOMATIC, BST_CHECKED);
		}
		else {
			auto weight = *m_Filter->weight.uint64;
			if (weight < 16) {
				CheckDlgButton(IDC_RANGE, BST_CHECKED);
				((CComboBox)GetDlgItem(IDC_WEIGHTRANGE)).SetCurSel((int)weight);
			}
			else {
				CheckDlgButton(IDC_SPECIFIC, BST_CHECKED);
				SetDlgItemText(IDC_WEIGHT, std::format(L"0x{:X}", weight).c_str());
			}
		}

	}

	return 0;
}

LRESULT CNewFilterDlg::OnCloseCmd(WORD, WORD wID, HWND, BOOL&) {
	if (wID == IDOK) {
		GetDlgItemText(IDC_NAME, m_Name);
		if (m_Name.IsEmpty()) {
			AtlMessageBox(m_hWnd, L"Filter name cannot be empty.", L"New Filter", MB_ICONWARNING);
			return 0;
		}

		CString key;
		GetDlgItemText(IDC_KEY, key);
		if (key.IsEmpty()) {
			AtlMessageBox(m_hWnd, L"Filter key cannot be empty.", L"New Filter", MB_ICONWARNING);
			return 0;
		}
		WFPEngine engine;
		if (!engine.Open())
			return ReportOpenEngineFailure();

		GetDlgItemText(IDC_DESC, m_Desc);
		m_Filter->displayData.name = m_Name.GetBuffer();
		m_Filter->displayData.description = m_Desc.GetBuffer();
		::CLSIDFromString(key, &m_Filter->filterKey);
		CComboBox cb(GetDlgItem(IDC_ACTION));
		m_Filter->action.type = (FWP_ACTION_TYPE)cb.GetItemData(cb.GetCurSel());
		if (m_Filter->action.type & FWP_ACTION_FLAG_CALLOUT) {
			CComboBox cb(GetDlgItem(IDC_CALLOUT));
			auto calloutId = (UINT)cb.GetItemData(cb.GetCurSel());

			auto callout = engine.GetCalloutById(calloutId);
			m_Filter->action.calloutKey = callout->calloutKey;
		}
		//
		// filter weight
		//
		if (IsDlgButtonChecked(IDC_AUTOMATIC) == BST_CHECKED) {
			m_Filter->weight.type = FWP_EMPTY;
		}
		else if (IsDlgButtonChecked(IDC_SPECIFIC) == BST_CHECKED) {
			CString text;
			GetDlgItemText(IDC_WEIGHT, text);
			m_Filter->weight.type = FWP_UINT64;
			m_Weight = wcstoull(text, nullptr, 0);
			m_Filter->weight.uint64 = &m_Weight;
		}
		else {
			ATLASSERT(IsDlgButtonChecked(IDC_RANGE) == BST_CHECKED);
			auto cb = (CComboBox)GetDlgItem(IDC_WEIGHTRANGE);
			int range = (int)cb.GetItemData(cb.GetCurSel());
			m_Filter->weight.type = FWP_UINT8;
			m_Filter->weight.int32 = range;
		}

		//
		// get provider
		//
		cb = (CComboBox)GetDlgItem(IDC_PROVIDER);
		if (cb.GetCurSel()) {
			CString text;
			cb.GetLBText(cb.GetCurSel(), text);
			m_ProviderKey = StringHelper::ExtractGuid(text);
			ATLASSERT(m_ProviderKey != GUID_NULL);
			m_Filter->providerKey = &m_ProviderKey;
		}
		//
		// get layer and sublayer
		//
		cb = (CComboBox)GetDlgItem(IDC_LAYER);
		m_Filter->layerKey = engine.GetLayerById((UINT16)cb.GetItemData(cb.GetCurSel()))->layerKey;
		cb = (CComboBox)GetDlgItem(IDC_SUBLAYER);
		CString text;
		cb.GetLBText(cb.GetCurSel(), text);
		m_Filter->subLayerKey = StringHelper::ExtractGuid(text);
	}
	EndDialog(wID);

	return 0;
}

LRESULT CNewFilterDlg::OnGenerateKey(WORD, WORD wID, HWND, BOOL&) {
	GUID guid;
	::CoCreateGuid(&guid);
	WCHAR sguid[64];
	::StringFromGUID2(guid, sguid, _countof(sguid));
	SetDlgItemText(IDC_KEY, sguid);

	return 0;
}

LRESULT CNewFilterDlg::OnLayerProperties(WORD, WORD wID, HWND, BOOL&) {
	auto cb = (CComboBox)GetDlgItem(IDC_LAYER);
	auto selected = cb.GetCurSel();
	auto id = (UINT16)cb.GetItemData(selected);

	WFPEngine engine;
	engine.Open();
	WFPHelper::ShowLayerProperties(engine, *engine.GetLayerById(id));

	return 0;
}

LRESULT CNewFilterDlg::OnSublayerProperties(WORD, WORD wID, HWND, BOOL&) {
	auto cb = (CComboBox)GetDlgItem(IDC_SUBLAYER);
	auto selected = cb.GetCurSel();
	CString text;
	cb.GetLBText(selected, text);
	auto guid = StringHelper::ExtractGuid(text);

	WFPEngine engine;
	engine.Open();
	WFPHelper::ShowSublayerProperties(engine, *engine.GetSublayerByKey(guid));

	return 0;
}

LRESULT CNewFilterDlg::OnProviderProperties(WORD, WORD wID, HWND, BOOL&) {
	auto cb = (CComboBox)GetDlgItem(IDC_PROVIDER);
	auto selected = cb.GetCurSel();
	if (selected == 0)
		return 0;

	CString text;
	cb.GetLBText(selected, text);
	auto guid = StringHelper::ExtractGuid(text);

	WFPEngine engine;
	engine.Open();
	WFPHelper::ShowProviderProperties(engine, *engine.GetProviderByKey(guid));

	return 0;
}
