#include "pch.h"
#include "SublayersView.h"
#include "StringHelper.h"

CSublayersView::CSublayersView(IMainFrame* frame, WFPEngine& engine) : CFrameView(frame), m_Engine(engine) {
}

LRESULT CSublayersView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	m_hWndClient = m_List.Create(m_hWnd, rcDefault, nullptr,
		WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | LVS_OWNERDATA | LVS_REPORT | LVS_SHOWSELALWAYS);
	m_List.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"Layer Key", 0, 250, ColumnType::Key);
	cm->AddColumn(L"Name", 0, 180, ColumnType::Name);
	cm->AddColumn(L"Flags", LVCFMT_RIGHT, 80, ColumnType::Flags);
	cm->AddColumn(L"Weight", LVCFMT_RIGHT, 80, ColumnType::Weight);
	cm->AddColumn(L"Provider", 0, 180, ColumnType::Provider);
	cm->AddColumn(L"Description", 0, 180, ColumnType::Desc);

	Refresh();

	return 0;
}

LRESULT CSublayersView::OnRefresh(WORD, WORD, HWND, BOOL&) {
	Refresh();
	return 0;
}

void CSublayersView::Refresh() {
	m_Layers = m_Engine.EnumSubLayers<SubLayerInfo>();
	m_List.SetItemCountEx((int)m_Layers.size(), LVSICF_NOSCROLL);
}

CString CSublayersView::GetColumnText(HWND, int row, int col) {
	auto& info = m_Layers[row];
	switch (GetColumnManager(m_List)->GetColumnTag<ColumnType>(col)) {
		case ColumnType::Key: return StringHelper::GuidToString(info.SubLayerKey);
		case ColumnType::Name: return info.Name.c_str();
		case ColumnType::Desc: return info.Desc.c_str();
		case ColumnType::Flags: return std::to_wstring(info.Flags).c_str();
		case ColumnType::Weight: return std::to_wstring(info.Weight).c_str();
		case ColumnType::Provider:
			if (info.ProviderName.IsEmpty()) {
				if (info.ProviderKey != GUID_NULL) {
					auto provider = m_Engine.GetProviderByKey(info.ProviderKey);
					if (provider && !provider.Name.empty() && provider.Name[0] != L'@')
						info.ProviderName = provider.Name.c_str();
					else
						info.ProviderName = StringHelper::GuidToString(info.ProviderKey);
				}
			}
			return info.ProviderName;

	}
	return CString();
}
