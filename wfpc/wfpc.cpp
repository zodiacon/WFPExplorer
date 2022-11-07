// wfpc.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "..\WFPCore\WFPEngine.h"

#pragma comment(lib, "Fwpuclnt.lib")
#pragma comment(lib, "Shlwapi.lib")

std::wstring GuidToString(GUID const& guid) {
	WCHAR sguid[64];
	return ::StringFromGUID2(guid, sguid, _countof(sguid)) ? sguid : L"";
}

void DisplaySessions(std::vector<WFPSessionInfo> const& sessions) {
	printf("Total sessions: %u\n", (UINT32)sessions.size());
	int n = printf("%-39s %-6s %-28s %6s %-28s %s\n",
		"Key", "PID", "User name", "Flags", "Name", "Description");
	printf("%s\n", std::string(n, '-').c_str());

	for (auto& session : sessions) {
		printf("%ws %6u %-28ws %6X %-28ws %ws\n",
			GuidToString(session.SessionKey).c_str(),
			session.ProcessId,
			session.UserName.c_str(),
			session.Flags,
			session.Name.c_str(),
			session.Desc.c_str());
	}
}

void DisplayProviders(std::vector<WFPProviderInfo> const& providers) {
	printf("Total providers: %u\n", (UINT32)providers.size());
	for (auto& p : providers) {
		printf("%-39ws %-20ws %-20ws %-20ws\n",
			GuidToString(p.ProviderKey).c_str(),
			p.ServiceName.c_str(),
			p.Name.c_str(),
			p.Desc.c_str());
	}
}

void DisplayFilters(std::vector<WFPFilterInfo> const& filters) {
	printf("Total filters: %u\n", (UINT32)filters.size());
	for (auto& f : filters) {
		printf("%ws %ws %-20ws %-20ws\n",
			GuidToString(f.FilterKey).c_str(),
			GuidToString(f.LayerKey).c_str(),
			f.Name.c_str(),
			f.Desc.c_str());
	}
}

void DisplayCallouts(std::vector<WFPCalloutInfo> const& callouts) {
	printf("Total callouts: %u\n", (UINT32)callouts.size());
	for (auto& c : callouts) {
		printf("%ws %ws %-20ws %-20ws\n",
			GuidToString(c.CalloutKey).c_str(),
			GuidToString(c.ProviderKey).c_str(),
			c.Name.c_str(),
			c.Desc.c_str());
	}
}

void DisplayLayers(std::vector<WFPLayerInfo> const& layers) {
	printf("Total layers: %u\n", (UINT32)layers.size());
	for (auto& layer : layers) {
		printf("%ws %5u %ws %2d fields %-20ws %-20ws\n",
			GuidToString(layer.LayerKey).c_str(),
			layer.LayerId,
			GuidToString(layer.DefaultSubLayerKey).c_str(),
			(int)layer.Fields.size(),
			layer.Name.c_str(),
			layer.Desc.c_str());
	}
}

bool IsRunningElevated() {
	HANDLE hToken;
	if (!::OpenProcessToken(::GetCurrentProcess(), TOKEN_QUERY, &hToken))
		return false;

	TOKEN_ELEVATION te;
	DWORD len;
	bool elevated = false;
	if (::GetTokenInformation(hToken, TokenElevation, &te, sizeof(te), &len)) {
		elevated = te.TokenIsElevated ? true : false;
	}
	::CloseHandle(hToken);
	return elevated;
}

int main(int argc, const char* argv[]) {
	if (argc < 2) {
		printf("Usage: wfpc <s[essions] | c[allouts] | f[ilters] | p[roviders] | l[ayers]>\n");
		return 0;
	}

	if (!IsRunningElevated()) {
		printf("Please run with admin rights.\n");
		return 0;
	}

	WFPEngine engine;
	if (!engine.Open()) {
		printf("Error accessing engine (%u)\n", engine.LastError());
		return 1;
	}

	IPSEC_STATISTICS1 stats;
	::IPsecGetStatistics(engine.Handle(), &stats);

	switch (argv[1][0]) {
		case 's': case 'S':
			DisplaySessions(engine.EnumSessions());
			break;

		case 'p': case 'P':
			DisplayProviders(engine.EnumProviders());
			break;

		case 'l': case 'L':
			DisplayLayers(engine.EnumLayers());
			break;

		case 'f': case 'F':
			DisplayFilters(engine.EnumFilters());
			break;

		case 'c': case 'C':
			DisplayCallouts(engine.EnumCallouts());
			break;

		default:
			printf("Unknown option.\n");
			break;
	}

	return 0;
}

