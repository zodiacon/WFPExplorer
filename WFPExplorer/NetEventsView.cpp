#include "pch.h"
#include <IconHelper.h>
#include "NetEventsView.h"
#include <WFPEnumerators.h>
#include <atltime.h>
#include "StringHelper.h"
#include <SortHelper.h>
#include <ClipboardHelper.h>
#include "PropertiesListDlg.h"

namespace {
	// a Win32 error (or NTSTATUS) code and its message
	CString ErrorText(DWORD code, bool ntstatus) {
		if (code == 0)
			return L"";
		CString text;
		text.Format(L"0x%08X", code);
		PWSTR message = nullptr;
		auto flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS |
			(ntstatus ? FORMAT_MESSAGE_FROM_HMODULE : FORMAT_MESSAGE_FROM_SYSTEM);
		if (::FormatMessage(flags, ntstatus ? ::GetModuleHandle(L"ntdll.dll") : nullptr, code, 0, (PWSTR)&message, 0, nullptr) && message) {
			CString msg(message);
			msg.TrimRight(L"\r\n .");
			text += L" (" + msg + L")";
			::LocalFree(message);
		}
		return text;
	}

	// msFwpDirection of classify events: MS_FWP_DIRECTION_IN/OUT
	CString MsFwpDirection(UINT32 direction) {
		switch (direction) {
			case 0x3900: return L"In";
			case 0x3901: return L"Out";
		}
		CString text;
		text.Format(L"0x%X", direction);
		return text;
	}

	// a firewall profile (FW_PROFILE_TYPE flags)
	CString Profile(UINT32 profile) {
		if (profile == 0)
			return L"";
		CString text;
		static const std::pair<UINT32, PCWSTR> names[] = { { 1, L"Domain" }, { 2, L"Private" }, { 4, L"Public" } };
		for (auto& [bit, name] : names) {
			if (profile & bit) {
				if (!text.IsEmpty())
					text += L", ";
				text += name;
				profile &= ~bit;
			}
		}
		if (profile) {
			CString rest;
			rest.Format(L"0x%X", profile);
			if (!text.IsEmpty())
				text += L", ";
			text += rest;
		}
		return text;
	}

	PCWSTR Capability(FWPM_APPC_NETWORK_CAPABILITY_TYPE type) {
		switch (type) {
			case FWPM_APPC_NETWORK_CAPABILITY_INTERNET_CLIENT: return L"Internet Client";
			case FWPM_APPC_NETWORK_CAPABILITY_INTERNET_CLIENT_SERVER: return L"Internet Client/Server";
			case FWPM_APPC_NETWORK_CAPABILITY_INTERNET_PRIVATE_NETWORK: return L"Private Network";
		}
		return L"";
	}

	CString Mac(FWP_BYTE_ARRAY6 const& mac) {
		CString text;
		auto& b = mac.byteArray6;
		text.Format(L"%02X-%02X-%02X-%02X-%02X-%02X", b[0], b[1], b[2], b[3], b[4], b[5]);
		return text;
	}

	CString Hex(UINT64 value) {
		CString text;
		text.Format(L"0x%llX", value);
		return text;
	}

	// the name of an enum value, from the names of the values 0, 1, 2...; the number for another value
	template<typename E>
	CString EnumName(E value, std::initializer_list<PCWSTR> names) {
		auto index = (size_t)value;
		if (index < names.size())
			return names.begin()[index];
		return std::to_wstring(index).c_str();
	}

	CString Number(UINT64 value) {
		return std::to_wstring(value).c_str();
	}

	// a certificate hash; empty if there's none
	CString Hash(UINT8 const (&hash)[20]) {
		if (std::ranges::all_of(hash, [](auto b) { return b == 0; }))
			return L"";
		CString text;
		for (auto b : hash)
			text.AppendFormat(L"%02X", b);
		return text;
	}

	CString FailurePoint(IPSEC_FAILURE_POINT point) {
		return EnumName(point, { L"None", L"Local", L"Peer" });
	}

	CString KeyModule(IKEEXT_KEY_MODULE_TYPE type) {
		return EnumName(type, { L"IKE", L"AuthIP", L"IKEv2" });
	}

	CString SaRole(IKEEXT_SA_ROLE role) {
		return EnumName(role, { L"Initiator", L"Responder" });
	}

	CString AuthMethod(IKEEXT_AUTHENTICATION_METHOD_TYPE method) {
		return EnumName(method, { L"Preshared Key", L"Certificate", L"Kerberos", L"Anonymous", L"SSL", L"NTLM v2", L"IPv6 CGA",
			L"Certificate (ECDSA P-256)", L"Certificate (ECDSA P-384)", L"SSL (ECDSA P-256)", L"SSL (ECDSA P-384)", L"EAP", L"Reserved" });
	}

	// a blob that holds a UTF-16 string (the app ID is a path, the effective name a name)
	CString BlobString(FWP_BYTE_BLOB const& blob) {
		if (blob.data == nullptr || blob.size < sizeof(WCHAR))
			return L"";
		CString text((PCWSTR)blob.data, (int)(blob.size / sizeof(WCHAR)));
		text.TrimRight(L'\0');
		return text;
	}
}

CNetEventsView::CNetEventsView(IMainFrame* frame, WFPEngine& engine) : CGenericListViewBase(frame), m_Engine(engine) {
}

void CNetEventsView::Refresh() {
	m_Events = WFPNetEventEnumerator(m_Engine.Handle()).Next<NetEventInfo>(1024);
	for (auto& info : m_Events)
		info.Details = GetDetails(info.Data);
	// filters and layers may have come and gone
	m_FilterNames.clear();
	m_LayerNames.clear();
	Sort(m_List);
	m_List.SetItemCountEx((int)m_Events.size(), LVSICF_NOSCROLL);
}

//
// each type of event has its own structure; the properties that some of them share are collected here
//
CNetEventsView::EventDetails CNetEventsView::GetDetails(FWPM_NET_EVENT const* e) {
	EventDetails d;
	// classify drop, classify allow and MAC classify drop have these in common
	auto classify = [&](auto c) {
		d.HasFilter = d.HasLayer = d.HasClassify = true;
		d.FilterId = c->filterId;
		d.LayerId = c->layerId;
		d.ReauthReason = c->reauthReason;
		d.OriginalProfile = c->originalProfile;
		d.CurrentProfile = c->currentProfile;
		d.Direction = MsFwpDirection(c->msFwpDirection);
		d.Loopback = c->isLoopback;
	};
	auto capability = [&](auto c) {
		d.HasFilter = true;
		d.FilterId = c->filterId;
		d.Loopback = c->isLoopback;
		d.Capability = Capability(c->networkCapabilityId);
	};
	auto ipsecDirection = [](FWP_DIRECTION direction) { return direction == FWP_DIRECTION_INBOUND ? L"In" : L"Out"; };

	switch (e->type) {
		case FWPM_NET_EVENT_TYPE_CLASSIFY_DROP:
			if (e->classifyDrop)
				classify(e->classifyDrop);
			break;

		case FWPM_NET_EVENT_TYPE_CLASSIFY_ALLOW:
			if (e->classifyAllow)
				classify(e->classifyAllow);
			break;

		case FWPM_NET_EVENT_TYPE_CLASSIFY_DROP_MAC:
			if (auto m = e->classifyDropMac) {
				classify(m);
				d.LocalMac = Mac(m->localMacAddr);
				d.RemoteMac = Mac(m->remoteMacAddr);
			}
			break;

		case FWPM_NET_EVENT_TYPE_CAPABILITY_DROP:
			if (e->capabilityDrop)
				capability(e->capabilityDrop);
			break;

		case FWPM_NET_EVENT_TYPE_CAPABILITY_ALLOW:
			if (e->capabilityAllow)
				capability(e->capabilityAllow);
			break;

		case FWPM_NET_EVENT_TYPE_IPSEC_KERNEL_DROP:
			if (auto i = e->ipsecDrop) {
				d.HasFilter = d.HasLayer = true;
				d.FilterId = i->filterId;
				d.LayerId = i->layerId;
				d.Direction = ipsecDirection(i->direction);
				d.Error = ErrorText(i->failureStatus, true);
				d.Spi = Hex(i->spi);
			}
			break;

		case FWPM_NET_EVENT_TYPE_IPSEC_DOSP_DROP:
			if (auto i = e->idpDrop) {
				d.Direction = ipsecDirection(i->direction);
				d.Error = ErrorText(i->failureStatus, true);
			}
			break;

		case FWPM_NET_EVENT_TYPE_IKEEXT_MM_FAILURE:
			if (auto f = e->ikeMmFailure) {
				d.HasFilter = true;
				d.FilterId = f->mmFilterId;
				d.Error = ErrorText(f->failureErrorCode, false);
			}
			break;

		case FWPM_NET_EVENT_TYPE_IKEEXT_QM_FAILURE:
			if (auto f = e->ikeQmFailure) {
				d.HasFilter = true;
				d.FilterId = f->qmFilterId;
				d.Error = ErrorText(f->failureErrorCode, false);
			}
			break;

		case FWPM_NET_EVENT_TYPE_IKEEXT_EM_FAILURE:
			if (auto f = e->ikeEmFailure) {
				d.HasFilter = true;
				d.FilterId = f->qmFilterId;
				d.Error = ErrorText(f->failureErrorCode, false);
			}
			break;

		case FWPM_NET_EVENT_TYPE_LPM_PACKET_ARRIVAL:
			if (e->lpmPacketArrival)
				d.Spi = Hex(e->lpmPacketArrival->spi);
			break;
	}
	return d;
}

CString const& CNetEventsView::GetFilterName(UINT64 id) {
	auto [it, added] = m_FilterNames.try_emplace(id);
	if (added) {
		// the filter may be gone (a dynamic filter, or deleted since)
		if (auto filter = m_Engine.GetFilterById(id, false); filter)
			it->second = StringHelper::ParseMUIString(filter->displayData.name);
	}
	return it->second;
}

CString const& CNetEventsView::GetLayerName(UINT16 id) {
	auto [it, added] = m_LayerNames.try_emplace(id);
	if (added) {
		if (auto layer = m_Engine.GetLayerById(id); layer)
			it->second = StringHelper::ParseMUIString(layer->displayData.name);
	}
	return it->second;
}

CString CNetEventsView::GetColumnText(HWND, int row, int col) {
	return GetText(m_Events[row], GetColumnManager(m_List)->GetColumnTag<ColumnType>(col));
}

CString CNetEventsView::GetText(NetEventInfo& info, ColumnType column) {
	auto e = info.Data;
	auto flags = e->header.flags;
	auto& d = info.Details;

	switch (column) {
		case ColumnType::FilterId: return d.HasFilter ? std::to_wstring(d.FilterId).c_str() : L"";
		case ColumnType::Filter: return d.HasFilter ? GetFilterName(d.FilterId) : CString();
		case ColumnType::Layer: return d.HasLayer ? GetLayerName(d.LayerId) : CString();
		case ColumnType::Direction: return d.Direction;
		case ColumnType::Loopback: return d.Loopback ? L"Yes" : L"";
		case ColumnType::OriginalProfile: return Profile(d.OriginalProfile);
		case ColumnType::CurrentProfile: return Profile(d.CurrentProfile);
		case ColumnType::ReauthReason: return d.HasClassify && d.ReauthReason ? Hex(d.ReauthReason) : CString();
		case ColumnType::Error: return d.Error;
		case ColumnType::Capability: return d.Capability;
		case ColumnType::Spi: return d.Spi;
		case ColumnType::LocalMac: return d.LocalMac;
		case ColumnType::RemoteMac: return d.RemoteMac;
		case ColumnType::EnterpriseId: return (flags & FWPM_NET_EVENT_FLAG_ENTERPRISE_ID_SET) && e->header.enterpriseId ? e->header.enterpriseId : L"";
		case ColumnType::PolicyFlags: return (flags & FWPM_NET_EVENT_FLAG_POLICY_FLAGS_SET) ? Hex(e->header.policyFlags) : CString();
		case ColumnType::EffectiveName: return (flags & FWPM_NET_EVENT_FLAG_EFFECTIVE_NAME_SET) ? BlobString(e->header.effectiveName) : CString();
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
			case ColumnType::FilterId: return SortHelper::Sort(ev1.Details.FilterId, ev2.Details.FilterId, asc);
		}
		// the others by their text
		return SortHelper::Sort(GetText(ev1, col), GetText(ev2, col), asc);
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
	// network events can't be deleted (the engine keeps them for a while, and there's no API to delete one)
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
		// the app ID is the application's path (a UTF-16 string)
		info.AppId = (info.Data->header.flags & FWPM_NET_EVENT_FLAG_APP_ID_SET) ? BlobString(info.Data->header.appId) : CString(L"");
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
	images.AddIcon(IconHelper::LoadCached(IDI_EVENT, 16));
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
	// what the event is about: the filter (and its layer) that decided, the direction, and the error of a failure
	cm->AddColumn(L"Direction", 0, 60, ColumnType::Direction);
	cm->AddColumn(L"Filter ID", LVCFMT_RIGHT, 70, ColumnType::FilterId, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Filter", 0, 220, ColumnType::Filter);
	cm->AddColumn(L"Layer", 0, 200, ColumnType::Layer);
	cm->AddColumn(L"Error", 0, 250, ColumnType::Error);
	cm->AddColumn(L"App ID", 0, 300, ColumnType::AppId);
	cm->AddColumn(L"User ID", 0, 180, ColumnType::UserId);
	cm->AddColumn(L"Package ID", 0, 180, ColumnType::PackageId);
	cm->AddColumn(L"Loopback", 0, 65, ColumnType::Loopback);
	cm->AddColumn(L"Original Profile", 0, 100, ColumnType::OriginalProfile);
	cm->AddColumn(L"Current Profile", 0, 100, ColumnType::CurrentProfile);
	cm->AddColumn(L"Reauth Reason", LVCFMT_RIGHT, 90, ColumnType::ReauthReason, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Capability", 0, 130, ColumnType::Capability);
	cm->AddColumn(L"Scope ID", LVCFMT_RIGHT, 80, ColumnType::ScopeId, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"SPI", LVCFMT_RIGHT, 90, ColumnType::Spi, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Local MAC", 0, 130, ColumnType::LocalMac, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Remote MAC", 0, 130, ColumnType::RemoteMac, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Enterprise ID", 0, 120, ColumnType::EnterpriseId);
	cm->AddColumn(L"Policy Flags", LVCFMT_RIGHT, 90, ColumnType::PolicyFlags, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Effective Name", 0, 150, ColumnType::EffectiveName);

	Refresh();

	return 0;
}

CPropertiesListDlg::Properties CNetEventsView::GetProperties(NetEventInfo& info) {
	CPropertiesListDlg::Properties props;
	auto add = [&](PCWSTR name, CString const& value) {
		if (!value.IsEmpty())
			props.push_back({ name, value });
	};

	// what the columns show, in their order
	auto cm = GetColumnManager(m_List);
	for (int i = 0; i < cm->GetCount(); i++) {
		auto& column = cm->GetColumn(i);
		if (auto text = GetText(info, (ColumnType)column.Tag); !text.IsEmpty())
			props.push_back({ column.Name, text });
	}

	//
	// and what they don't
	//
	auto e = info.Data;
	auto& header = e->header;
	if (header.flags & FWPM_NET_EVENT_FLAG_IP_VERSION_SET)
		add(L"IP Version", header.ipVersion == FWP_IP_VERSION_V4 ? L"IPv4" : header.ipVersion == FWP_IP_VERSION_V6 ? L"IPv6" : L"");
	static const std::pair<UINT32, PCWSTR> flagNames[] = {
		{ FWPM_NET_EVENT_FLAG_IP_PROTOCOL_SET, L"IP Protocol" },
		{ FWPM_NET_EVENT_FLAG_LOCAL_ADDR_SET, L"Local Address" },
		{ FWPM_NET_EVENT_FLAG_REMOTE_ADDR_SET, L"Remote Address" },
		{ FWPM_NET_EVENT_FLAG_LOCAL_PORT_SET, L"Local Port" },
		{ FWPM_NET_EVENT_FLAG_REMOTE_PORT_SET, L"Remote Port" },
		{ FWPM_NET_EVENT_FLAG_APP_ID_SET, L"App ID" },
		{ FWPM_NET_EVENT_FLAG_USER_ID_SET, L"User ID" },
		{ FWPM_NET_EVENT_FLAG_SCOPE_ID_SET, L"Scope ID" },
		{ FWPM_NET_EVENT_FLAG_IP_VERSION_SET, L"IP Version" },
		{ FWPM_NET_EVENT_FLAG_REAUTH_REASON_SET, L"Reauth Reason" },
		{ FWPM_NET_EVENT_FLAG_PACKAGE_ID_SET, L"Package ID" },
		{ FWPM_NET_EVENT_FLAG_ENTERPRISE_ID_SET, L"Enterprise ID" },
		{ FWPM_NET_EVENT_FLAG_POLICY_FLAGS_SET, L"Policy Flags" },
		{ FWPM_NET_EVENT_FLAG_EFFECTIVE_NAME_SET, L"Effective Name" },
	};
	CString flags;
	for (auto& [flag, name] : flagNames) {
		if (header.flags & flag) {
			if (!flags.IsEmpty())
				flags += L", ";
			flags += name;
		}
	}
	add(L"Fields Set", flags);

	auto principals = [&](PCWSTR local, PCWSTR remote) {
		add(L"Local Principal", local);
		add(L"Remote Principal", remote);
	};
	auto vswitch = [&](auto c) {
		if (auto id = BlobString(c->vSwitchId); !id.IsEmpty()) {
			add(L"vSwitch ID", id);
			add(L"vSwitch Source Port", Number(c->vSwitchSourcePort));
			add(L"vSwitch Destination Port", Number(c->vSwitchDestinationPort));
		}
	};

	switch (e->type) {
		case FWPM_NET_EVENT_TYPE_CLASSIFY_DROP:
			if (e->classifyDrop)
				vswitch(e->classifyDrop);
			break;

		case FWPM_NET_EVENT_TYPE_CLASSIFY_DROP_MAC:
			if (auto m = e->classifyDropMac) {
				add(L"Media Type", Number(m->mediaType));
				add(L"Interface Type", Number(m->ifType));
				add(L"Ether Type", Hex(m->etherType));
				add(L"VLAN Tag", Number(m->vlanTag));
				add(L"NDIS Port", Number(m->ndisPortNumber));
				add(L"Interface LUID", Hex(m->ifLuid));
				vswitch(m);
			}
			break;

		case FWPM_NET_EVENT_TYPE_IPSEC_DOSP_DROP:
			if (auto d = e->idpDrop) {
				bool v4 = d->ipVersion == FWP_IP_VERSION_V4;
				add(L"Public Host", v4 ? StringHelper::FormatIpv4Address(d->publicHostV4Addr) : StringHelper::FormatIpv6Address(d->publicHostV6Addr));
				add(L"Internal Host", v4 ? StringHelper::FormatIpv4Address(d->internalHostV4Addr) : StringHelper::FormatIpv6Address(d->internalHostV6Addr));
			}
			break;

		case FWPM_NET_EVENT_TYPE_IKEEXT_MM_FAILURE:
			if (auto f = e->ikeMmFailure) {
				add(L"Failure Point", FailurePoint(f->failurePoint));
				add(L"Keying Module", KeyModule(f->keyingModuleType));
				add(L"Main Mode State", EnumName(f->mmState, { L"None", L"SA Sent", L"SSPI Sent", L"Final", L"Final Sent", L"Complete" }));
				add(L"Role", SaRole(f->saRole));
				add(L"Authentication", AuthMethod(f->mmAuthMethod));
				add(L"Main Mode ID", Number(f->mmId));
				principals(f->localPrincipalNameForAuth, f->remotePrincipalNameForAuth);
				add(L"Certificate Hash", Hash(f->endCertHash));
				if (f->providerContextKey)
					add(L"Provider Context", StringHelper::GuidToString(*f->providerContextKey));
			}
			break;

		case FWPM_NET_EVENT_TYPE_IKEEXT_QM_FAILURE:
			if (auto f = e->ikeQmFailure) {
				add(L"Failure Point", FailurePoint(f->failurePoint));
				add(L"Keying Module", KeyModule(f->keyingModuleType));
				add(L"Quick Mode State", EnumName(f->qmState, { L"None", L"Initial", L"Final", L"Complete" }));
				add(L"Role", SaRole(f->saRole));
				add(L"Main Mode SA LUID", Hex(f->mmSaLuid));
				add(L"Main Mode Provider Context", StringHelper::GuidToString(f->mmProviderContextKey));
			}
			break;

		case FWPM_NET_EVENT_TYPE_IKEEXT_EM_FAILURE:
			if (auto f = e->ikeEmFailure) {
				add(L"Failure Point", FailurePoint(f->failurePoint));
				add(L"Extended Mode State", EnumName(f->emState, { L"None", L"Sent Attributes", L"SSPI Sent", L"Auth Complete", L"Final", L"Complete" }));
				add(L"Role", SaRole(f->saRole));
				add(L"Authentication", AuthMethod(f->emAuthMethod));
				add(L"Main Mode ID", Number(f->mmId));
				principals(f->localPrincipalNameForAuth, f->remotePrincipalNameForAuth);
				add(L"Certificate Hash", Hash(f->endCertHash));
			}
			break;
	}
	return props;
}

bool CNetEventsView::OnDoubleClickList(HWND, int row, int, POINT const&) {
	if (row < 0)
		return false;
	BOOL handled;
	OnProperties(0, ID_EDIT_PROPERTIES, nullptr, handled);
	return true;
}

LRESULT CNetEventsView::OnProperties(WORD, WORD, HWND, BOOL&) {
	int selected = m_List.GetNextItem(-1, LVNI_SELECTED);
	if (selected < 0)
		return 0;
	auto& info = m_Events[selected];
	CPropertiesListDlg dlg(StringHelper::NetEventTypeToString(info.Data->type) + CString(L" Event"), IDI_EVENT, GetProperties(info));
	dlg.DoModal(m_hWnd);
	return 0;
}

LRESULT CNetEventsView::OnCopy(WORD, WORD, HWND, BOOL&) {
	ClipboardHelper::CopyText(m_hWnd, ListViewHelper::GetSelectedRowsAsString(m_List, L","));
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
