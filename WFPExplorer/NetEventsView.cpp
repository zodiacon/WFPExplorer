#include "pch.h"
#include "NetEventsView.h"
#include <WFPEnumerators.h>
#include <atltime.h>
#include "StringHelper.h"
#include <SortHelper.h>

CNetEventsView::CNetEventsView(IMainFrame* frame, WFPEngine& engine) : CGenericListViewBase(frame), m_Engine(engine) {
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
		case ColumnType::LocalPort:
		{
			auto port = GetLocalPort(info);
			return port ? std::to_wstring(port).c_str() : L"";
		}

		case ColumnType::RemotePort: 
		{
			auto port = GetRemotePort(info);
			return port ? std::to_wstring(port).c_str() : L"";
		}

		case ColumnType::LocalAddress: return GetLocalAddress(info);
		case ColumnType::RemoteAddress: return GetRemoteAddress(info);
		case ColumnType::UserId: return GetUserId(info); 
		case ColumnType::PackageId: return GetPackageId(info);
		case ColumnType::AppId: return GetAppId(info);
	}
	return CString();
}

void CNetEventsView::DoSort(SortInfo const* si) {
	auto col = GetColumnManager(si->hWnd)->GetColumnTag<ColumnType>(si->SortColumn);
	auto asc = si->SortAscending;

	auto compare = [&](auto& ev1, auto& ev2) {
		auto e1 = ev1.Data, e2 = ev2.Data;
		switch (col) {
			case ColumnType::LocalAddress: return SortHelper::Sort(GetLocalAddress(ev1), GetLocalAddress(ev2), asc);
			case ColumnType::RemoteAddress: return SortHelper::Sort(GetRemoteAddress(ev1), GetRemoteAddress(ev2), asc);
			case ColumnType::LocalPort: return SortHelper::Sort(GetLocalPort(ev1), GetLocalPort(ev2), asc);
			case ColumnType::RemotePort: return SortHelper::Sort(GetRemotePort(ev1), GetRemotePort(ev2), asc);
			case ColumnType::Time: return SortHelper::Sort(*(LONGLONG*)&e1->header.timeStamp, *(LONGLONG*)&e2->header.timeStamp, asc);
			case ColumnType::Type: return SortHelper::Sort(StringHelper::NetEventTypeToString(e1->type), StringHelper::NetEventTypeToString(e2->type), asc);
			case ColumnType::AddressFamily: return SortHelper::Sort(
				StringHelper::AddressFamilyToString(e1->header.addressFamily), 
				StringHelper::AddressFamilyToString(e2->header.addressFamily), asc);
			case ColumnType::Protocol: return SortHelper::Sort(
				StringHelper::IpProtocolToString(e1->header.ipProtocol), 
				StringHelper::IpProtocolToString(e2->header.ipProtocol), asc);
			case ColumnType::ScopeId: return SortHelper::Sort(e1->header.scopeId, e2->header.scopeId, asc);
			case ColumnType::AppId: return SortHelper::Sort(GetAppId(ev1), GetAppId(ev2), asc);
		}
		return false;
	};
	std::ranges::sort(m_Events, compare);
}

int CNetEventsView::GetRowImage(HWND, int row, int col) const {
	return 0;
}

void CNetEventsView::OnStateChanged(HWND, int from, int to, UINT oldState, UINT newState) {
	if ((newState & LVIS_SELECTED) || (oldState & LVIS_SELECTED))
		UpdateUI();
}

CString CNetEventsView::GetDefaultSaveFile() const {
	return L"events.csv";
}

void CNetEventsView::UpdateUI() const {
	auto& ui = Frame()->UI();
	auto selected = m_List.GetSelectedCount();
	ui.UIEnable(ID_EDIT_COPY, selected > 0);
	ui.UIEnable(ID_EDIT_DELETE, selected > 0);
	ui.UIEnable(ID_EDIT_PROPERTIES, selected == 1);
}

CString const& CNetEventsView::GetLocalAddress(NetEventInfo& info) const {
	if (info.LocalAddress.IsEmpty()) {
		auto const& header = info.Data->header;
		auto flags = header.flags;
		if (flags & FWPM_NET_EVENT_FLAG_LOCAL_ADDR_SET)
			info.LocalAddress = header.ipVersion == FWP_IP_VERSION_V4
				? StringHelper::FormatIpv4Address(header.localAddrV4)
				: StringHelper::FormatIpv6Address(header.localAddrV6.byteArray16);
	}
	return info.LocalAddress;
}

CString const& CNetEventsView::GetRemoteAddress(NetEventInfo& info) const {
	if (info.RemoteAddress.IsEmpty()) {
		auto const& header = info.Data->header;
		auto flags = header.flags;
		if (flags & FWPM_NET_EVENT_FLAG_REMOTE_ADDR_SET)
			info.RemoteAddress = header.ipVersion == FWP_IP_VERSION_V4
				? StringHelper::FormatIpv4Address(header.remoteAddrV4)
				: StringHelper::FormatIpv6Address(header.remoteAddrV6.byteArray16);
	}
	return info.RemoteAddress;
}

UINT16 CNetEventsView::GetLocalPort(NetEventInfo& info) const {
	return (info.Data->header.flags & FWPM_NET_EVENT_FLAG_LOCAL_PORT_SET) ? info.Data->header.localPort : 0;
}

UINT16 CNetEventsView::GetRemotePort(NetEventInfo& info) const {
	return (info.Data->header.flags & FWPM_NET_EVENT_FLAG_REMOTE_PORT_SET) ? info.Data->header.remotePort : 0;
}

CString const& CNetEventsView::GetAppId(NetEventInfo& info) {
	if (info.AppId.IsEmpty()) {
		info.AppId = (info.Data->header.flags & FWPM_NET_EVENT_FLAG_APP_ID_SET) ?
			StringHelper::FormatBinary(info.Data->header.appId.data, min(16, info.Data->header.appId.size)) : CString(L"");
	}
	return info.AppId;
}

CString const& CNetEventsView::GetUserId(NetEventInfo& info) {
	if (info.UserId.IsEmpty()) {
		info.UserId = (info.Data->header.flags & FWPM_NET_EVENT_FLAG_USER_ID_SET) ? StringHelper::FormatSID(info.Data->header.userId) : CString(L"");
	}
	return info.UserId;
}

CString const& CNetEventsView::GetPackageId(NetEventInfo& info) {
	if (info.PackageId.IsEmpty()) {
		info.PackageId = (info.Data->header.flags & FWPM_NET_EVENT_FLAG_PACKAGE_ID_SET) ? StringHelper::FormatSID(info.Data->header.packageSid) : CString(L"");
	}
	return info.PackageId;
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
	cm->AddColumn(L"Time", 0, 160, ColumnType::Time, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Protocol", 0, 90, ColumnType::Protocol);
	cm->AddColumn(L"Address Family", 0, 70, ColumnType::AddressFamily);
	cm->AddColumn(L"Local Address", LVCFMT_RIGHT, 300, ColumnType::LocalAddress, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Local Port", LVCFMT_RIGHT, 70, ColumnType::LocalPort, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Remote Address", LVCFMT_RIGHT, 300, ColumnType::RemoteAddress, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Remote Port", LVCFMT_RIGHT, 70, ColumnType::RemotePort, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Scope ID", LVCFMT_RIGHT, 80, ColumnType::ScopeId, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"User ID", 0, 180, ColumnType::UserId);
	cm->AddColumn(L"App ID", 0, 180, ColumnType::AppId, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Package ID", 0, 180, ColumnType::PackageId);

	Refresh();

	return 0;
}

LRESULT CNetEventsView::OnRefresh(WORD, WORD, HWND, BOOL&) {
	Refresh();
	return 0;
}

LRESULT CNetEventsView::OnActivate(UINT, WPARAM active, LPARAM, BOOL&) {
	if(active)
		UpdateUI();
	return 0;
}
