#include "pch.h"
#include "SublayersView.h"
#include "StringHelper.h"
#include <SortHelper.h>
#include "resource.h"
#include <WFPEnumerators.h>

CSublayersView::CSublayersView(IMainFrame* frame, WFPEngine& engine) : CGenericListViewBase(frame), m_Engine(engine) {
}

LRESULT CSublayersView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	m_hWndClient = m_List.Create(m_hWnd, rcDefault, nullptr,
		WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | LVS_OWNERDATA | LVS_REPORT | LVS_SHOWSELALWAYS);
	m_List.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"Layer Key", 0, 300, ColumnType::Key, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Name", 0, 180, ColumnType::Name);
	cm->AddColumn(L"Flags", 0, 120, ColumnType::Flags);
	cm->AddColumn(L"Weight", LVCFMT_RIGHT, 80, ColumnType::Weight, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Provider Data", LVCFMT_RIGHT, 90, ColumnType::ProviderData);
	cm->AddColumn(L"Provider", 0, 180, ColumnType::Provider);
	cm->AddColumn(L"Description", 0, 180, ColumnType::Desc);

	CImageList images;
	images.Create(16, 16, ILC_COLOR32 | ILC_MASK, 1, 1);
	images.AddIcon(AtlLoadIconImage(IDI_SUBLAYER, 0, 16, 16));
	m_List.SetImageList(images, LVSIL_SMALL);

	Refresh();

	return 0;
}

LRESULT CSublayersView::OnRefresh(WORD, WORD, HWND, BOOL&) {
	Refresh();
	return 0;
}

void CSublayersView::Refresh() {
	m_Layers = WFPSubLayerEnumerator(m_Engine.Handle()).Next<SubLayerInfo>(512);
	m_List.SetItemCountEx((int)m_Layers.size(), LVSICF_NOSCROLL);
	Frame()->SetStatusText(6, std::format(L"{} Sublayers", m_Layers.size()).c_str());
}

CString CSublayersView::GetColumnText(HWND, int row, int col) {
	auto& sl = m_Layers[row];
	auto info = sl.Data;
	switch (GetColumnManager(m_List)->GetColumnTag<ColumnType>(col)) {
		case ColumnType::Key: return StringHelper::GuidToString(info->subLayerKey);
		case ColumnType::Name: return sl.Name();
		case ColumnType::Desc: return sl.Desc();
		case ColumnType::ProviderData: return info->providerData.size == 0 ? L"" : std::format(L"{} Bytes", info->providerData.size).c_str();
		case ColumnType::Flags:
			if (info->flags == 0)
				return L"0";
			return std::format(L"0x{:X} ({})", info->flags,
				(PCWSTR)StringHelper::WFPSubLayerFlagsToString(info->flags)).c_str();

		case ColumnType::Weight: return std::to_wstring(info->weight).c_str();
		case ColumnType::Provider:
			if (sl.ProviderName.IsEmpty()) {
				if (info->providerKey) {
					auto provider = m_Engine.GetProviderByKey(*info->providerKey);
					if (provider)
						sl.ProviderName = StringHelper::ParseMUIString(provider->displayData.name);
					else
						sl.ProviderName = StringHelper::GuidToString(*info->providerKey);
				}
			}
			return sl.ProviderName;

	}
	return CString();
}

void CSublayersView::DoSort(SortInfo const* si) {
	auto col = GetColumnManager(m_List)->GetColumnTag<ColumnType>(si->SortColumn);
	auto asc = si->SortAscending;

	auto compare = [&](auto& sl1, auto& sl2) {
		auto l1 = sl1.Data, l2 = sl2.Data;
		switch (col) {
			case ColumnType::Key: return SortHelper::Sort(StringHelper::GuidToString(l1->subLayerKey), StringHelper::GuidToString(l2->subLayerKey), asc);
			case ColumnType::Name: return SortHelper::Sort(sl1.Name(), sl2.Name(), asc);
			case ColumnType::Desc: return SortHelper::Sort(sl1.Desc(), sl2.Desc(), asc);
			case ColumnType::Flags: return SortHelper::Sort(l1->flags, l2->flags, asc);
			case ColumnType::Provider: return SortHelper::Sort(sl1.ProviderName, sl2.ProviderName, asc);
			case ColumnType::Weight: return SortHelper::Sort(l1->weight, l2->weight, asc);
			case ColumnType::ProviderData: return SortHelper::Sort(l1->providerData.size, l2->providerData.size, asc);
		}
		return false;
	};
	std::ranges::sort(m_Layers, compare);
}

int CSublayersView::GetSaveColumnRange(HWND, int&) const {
	return 1;
}

int CSublayersView::GetRowImage(HWND, int row, int col) const {
	return 0;
}

CString CSublayersView::GetDefaultSaveFile() const {
	return L"sublayers.csv";
}

CString const& CSublayersView::SubLayerInfo::Name() const {
	if (m_Name.IsEmpty())
		m_Name = StringHelper::ParseMUIString(Data->displayData.name);
	return m_Name;
}

CString const& CSublayersView::SubLayerInfo::Desc() const {
	if (m_Desc.IsEmpty())
		m_Desc = StringHelper::ParseMUIString(Data->displayData.description);
	return m_Desc;
}
