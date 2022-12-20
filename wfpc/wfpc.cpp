// wfpc.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "..\WFPCore\WFPEngine.h"
#include "..\WFPCore\WFPEnumerators.h"

#pragma comment(lib, "Fwpuclnt.lib")
#pragma comment(lib, "Shlwapi.lib")

std::wstring GuidToString(GUID const& guid) {
	WCHAR sguid[64];
	return ::StringFromGUID2(guid, sguid, _countof(sguid)) ? sguid : L"";
}

void DisplaySessions(WFPEngine& engine) {
	WFPSessionEnumerator enumerator(engine.Handle());
	auto sessions = enumerator.Next(256);
	printf("Total sessions: %u\n", (UINT32)sessions.size());
	int n = printf("%-39s %-6s %-28s %6s %-28s %s\n",
		"Key", "PID", "User name", "Flags", "Name", "Description");
	printf("%s\n", std::string(n, '-').c_str());

	for (auto session : sessions) {
		printf("%ws %6u %-28ws %6X %-28ws %ws\n",
			GuidToString(session->sessionKey).c_str(),
			session->processId,
			session->username,
			session->flags,
			session->displayData.name,
			session->displayData.description);
	}
}

void DisplayFilters(WFPEngine& engine) {
	WFPFilterEnumerator enumerator(engine.Handle());
	auto filters = enumerator.Next(8192);
	printf("Total filters: %u\n", (UINT32)filters.size());
	for (auto f : filters) {
		printf("%ws %ws %-20ws %-20ws\n",
			GuidToString(f->filterKey).c_str(),
			GuidToString(f->layerKey).c_str(),
			f->displayData.name,
			f->displayData.description);
	}
}

void DisplayCallouts(WFPEngine& engine) {
	WFPCalloutEnumerator enumerator(engine.Handle());
	auto callouts = enumerator.Next(1024);
	printf("Total callouts: %u\n", (UINT32)callouts.size());
	for (auto c : callouts) {
		printf("%ws %ws %-20ws %-20ws\n",
			GuidToString(c->calloutKey).c_str(),
			c->providerKey ? GuidToString(*c->providerKey).c_str() : L"",
			c->displayData.name,
			c->displayData.description);
	}
}

void DisplayLayers(WFPEngine& engine) {
	WFPLayerEnumerator enumerator(engine.Handle());
	auto layers = enumerator.Next(1024);
	printf("Total layers: %u\n", (UINT32)layers.size());
	for (auto layer : layers) {
		printf("%ws %5u %ws %2d fields %-20ws %-20ws\n",
			GuidToString(layer->layerKey).c_str(),
			layer->layerId,
			GuidToString(layer->defaultSubLayerKey).c_str(),
			layer->numFields,
			layer->displayData.name,
			layer->displayData.description);
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

	WFPConnectionEnumerator enumerator(engine.Handle());
	auto conns = enumerator.Next(256);

	switch (argv[1][0]) {
		case 's': case 'S':
			DisplaySessions(engine);
			break;

		case 'p': case 'P':
			//DisplayProviders(engine);
			break;

		case 'l': case 'L':
			DisplayLayers(engine);
			break;

		case 'f': case 'F':
			DisplayFilters(engine);
			break;

		case 'c': case 'C':
			DisplayCallouts(engine);
			break;

		default:
			printf("Unknown option.\n");
			break;
	}

	return 0;
}

