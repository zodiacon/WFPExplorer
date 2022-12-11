#include "pch.h"
#include "NetEventsView.h"
#include <Enumerators.h>
#include <atltime.h>
#include "StringHelper.h"

CNetEventsView::CNetEventsView(IMainFrame* frame, WFPEngine& engine) : CFrameView(frame), m_Engine(engine) {
}

void CNetEventsView::Refresh() {
	m_Events = WFPNetEventEnumerator(m_Engine.Handle()).Next<NetEventInfo>(1024);
	Sort(m_List);
	m_List.SetItemCountEx((int)m_Events.size(), LVSICF_NOSCROLL);
}

CString CNetEventsView::GetColumnText(HWND, int row, int col) {
	auto& info = m_Events[row];
	auto e = info.Data;
	auto flags = e->header.flags;

	switch (GetColumnManager(m_List)->GetColumnTag<ColumnType>(col)) {
		case ColumnType::Type: return StringHelper::NetEventTypeToString(e->type);
		case ColumnType::Time: return CTime(e->header.timeStamp).Format(L"%x %X");
		case ColumnType::AddressFamily: return StringHelper::AddressFamilyToString(e->header.addressFamily);
		case ColumnType::Protocol: return StringHelper::IpProtocolToString(e->header.ipProtocol);
		case ColumnType::ScopeId: return (flags & FWPM_NET_EVENT_FLAG_SCOPE_ID_SET) ? std::to_wstring(e->header.scopeId).c_str() : L"";
		case ColumnType::LocalPort: return (flags & FWPM_NET_EVENT_FLAG_LOCAL_PORT_SET) ? std::to_wstring(e->header.localPort).c_str() : L"";
		case ColumnType::RemotePort: return (flags & FWPM_NET_EVENT_FLAG_REMOTE_PORT_SET) ? std::to_wstring(e->header.remotePort).c_str() : L"";
		case ColumnType::LocalAddress: 
			if (e->header.ipVersion == FWP_IP_VERSION_NONE || (flags & FWPM_NET_EVENT_FLAG_LOCAL_ADDR_SET) == 0)
				return L"";
			return e->header.ipVersion == FWP_IP_VERSION_V4
				? StringHelper::FormatIpv4Address(e->header.localAddrV4) 
				: StringHelper::FormatIpv6Address(e->header.localAddrV6.byteArray16);
		
		case ColumnType::RemoteAddress:
			if (e->header.ipVersion == FWP_IP_VERSION_NONE || (flags & FWPM_NET_EVENT_FLAG_REMOTE_ADDR_SET) == 0)
				return L"";
			return e->header.ipVersion == FWP_IP_VERSION_V4
				? StringHelper::FormatIpv4Address(e->header.remoteAddrV4) 
				: StringHelper::FormatIpv6Address(e->header.remoteAddrV6.byteArray16);

		case ColumnType::UserId: return (flags & FWPM_NET_EVENT_FLAG_USER_ID_SET) ? StringHelper::FormatSID(e->header.userId) : CString(L"");
		case ColumnType::PackageId: return (flags & FWPM_NET_EVENT_FLAG_PACKAGE_ID_SET) ? StringHelper::FormatSID(e->header.packageSid) : CString(L"");
		case ColumnType::AppId:
			return (e->header.flags & FWPM_NET_EVENT_FLAG_APP_ID_SET) ? 
				StringHelper::FormatBinary(e->header.appId.data, min(16, e->header.appId.size)) : CString(L"");
	}
	return CString();
}

void CNetEventsView::DoSort(SortInfo const* si) {
}

int CNetEventsView::GetRowImage(HWND, int row, int col) const {
	return 0;
}

void CNetEventsView::UpdateUI() {
}

LRESULT CNetEventsView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	m_hWndClient = m_List.Create(m_hWnd, rcDefault, nullptr,
		WS_CHILD | WS_VISIBLE | LVS_OWNERDATA | LVS_REPORT);
	m_List.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_HEADERDRAGDROP);
	CImageList images;
	images.Create(16, 16, ILC_COLOR32 | ILC_MASK, 1, 1);
	images.AddIcon(AtlLoadIconImage(IDI_EVENT, 0, 16, 16));
	m_List.SetImageList(images, LVSIL_SMALL);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"Type", 0, 150, ColumnType::Type);
	cm->AddColumn(L"Time", 0, 130, ColumnType::Time);
	cm->AddColumn(L"Protocol", 0, 90, ColumnType::Protocol);
	cm->AddColumn(L"Address Family", 0, 70, ColumnType::AddressFamily);
	cm->AddColumn(L"Local Address", LVCFMT_RIGHT, 170, ColumnType::LocalAddress);
	cm->AddColumn(L"Local Port", LVCFMT_RIGHT, 80, ColumnType::LocalPort);
	cm->AddColumn(L"Remote Address", LVCFMT_RIGHT, 170, ColumnType::RemoteAddress);
	cm->AddColumn(L"Remote Port", LVCFMT_RIGHT, 80, ColumnType::RemotePort);
	cm->AddColumn(L"Scope ID", LVCFMT_RIGHT, 80, ColumnType::ScopeId);
	cm->AddColumn(L"User ID", 0, 180, ColumnType::UserId);
	cm->AddColumn(L"App ID", 0, 180, ColumnType::AppId);
	cm->AddColumn(L"Package ID", 0, 180, ColumnType::PackageId);

	Refresh();

	return 0;
}

LRESULT CNetEventsView::OnRefresh(WORD, WORD, HWND, BOOL&) {
	Refresh();
	return 0;
}

LRESULT CNetEventsView::OnActivate(UINT, WPARAM, LPARAM, BOOL&) {
	UpdateUI();
	return 0;
}
