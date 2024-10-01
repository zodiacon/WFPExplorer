#include "pch.h"
#include "resource.h"
#include "NewSubLayerDlg.h"
#include "WFPHelper.h"
#include "StringHelper.h"
#include <WFPEngine.h>
#include <WFPEnumerators.h>

CNewSubLayerDlg::CNewSubLayerDlg(FWPM_SUBLAYER* sublayer) : m_SubLayer(sublayer) {
}

bool CNewSubLayerDlg::IsNewSubLayer() const {
	return m_SubLayer->subLayerKey == GUID_NULL;
}

bool CNewSubLayerDlg::UpdateSubLayer() {
	GetDlgItemText(IDC_NAME, m_Name);
	if (m_Name.IsEmpty()) {
		AtlMessageBox(m_hWnd, L"Sublayer name cannot be empty.", L"New Sublayer", MB_ICONWARNING);
		return false;
	}

	CString key;
	GetDlgItemText(IDC_KEY, key);
	GetDlgItemText(IDC_DESC, m_Desc);
	m_SubLayer->displayData.name = m_Name.GetBuffer();
	m_SubLayer->displayData.description = m_Desc.GetBuffer();
	::CLSIDFromString(key, &m_SubLayer->subLayerKey);

	GetDlgItemText(IDC_WEIGHT, key);
	m_SubLayer->weight = (UINT16)wcstol(key, nullptr, 0);

	//
	// get provider
	//
	auto cb = (CComboBox)GetDlgItem(IDC_PROVIDER);
	if (cb.GetCurSel()) {
		CString text;
		cb.GetLBText(cb.GetCurSel(), text);
		m_ProviderKey = StringHelper::ExtractGuid(text);
		ATLASSERT(m_ProviderKey != GUID_NULL);
		m_SubLayer->providerKey = &m_ProviderKey;
	}

	return true;
}

LRESULT CNewSubLayerDlg::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
	using namespace std;
	using namespace std::views;
	using namespace std::ranges;

	SetDialogIcon(IDI_SUBLAYER);
	WFPEngine engine;
	if (engine.Open()) {
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
			if (selected < 0 && m_SubLayer->providerKey && IsNewSubLayer() && p->providerKey == *m_SubLayer->providerKey)
				selected = i;
			i++;

			return Data{ p->displayData.name + CString(" (") + StringHelper::GuidToString(p->providerKey) + L")", i - 1 };
			}) | to<vector>();

		data.insert(data.begin(), { L"(None)", -1 });
		InitCombo(IDC_PROVIDER, data.data(), (int)data.size(), selected);
	}

	if (IsNewSubLayer()) {
		SetDlgItemInt(IDC_WEIGHT, 0);
	}
	else {
		SetDlgItemText(IDC_NAME, m_SubLayer->displayData.name);
		SetDlgItemText(IDC_DESC, m_SubLayer->displayData.description);
		SetDlgItemText(IDC_KEY, StringHelper::GuidToString(m_SubLayer->subLayerKey));
		if (m_SubLayer->flags & FWPM_SUBLAYER_FLAG_PERSISTENT)
			CheckDlgButton(IDC_PERSISTENT, BST_CHECKED);
	}

	return 0;
}

LRESULT CNewSubLayerDlg::OnCloseCmd(WORD, WORD wID, HWND, BOOL&) {
	if (wID == IDOK) {
		if (!UpdateSubLayer())
			return 0;
	}
	EndDialog(wID);
	return 0;
}

LRESULT CNewSubLayerDlg::OnGenerateKey(WORD, WORD wID, HWND, BOOL&) {
	GUID guid;
	::CoCreateGuid(&guid);
	WCHAR sguid[64];
	::StringFromGUID2(guid, sguid, _countof(sguid));
	SetDlgItemText(IDC_KEY, sguid);

	return 0;
}

LRESULT CNewSubLayerDlg::OnProviderProperties(WORD, WORD wID, HWND, BOOL&) {
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
