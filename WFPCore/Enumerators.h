#pragma once

#include "WFPEnumerator.h"

struct WFPNetEventEnumerator : WFPEnumerator<
	decltype(&::FwpmNetEventCreateEnumHandle),
	decltype(&::FwpmNetEventDestroyEnumHandle),
	decltype(&::FwpmNetEventEnum),
	FWPM_NET_EVENT> {
	explicit WFPNetEventEnumerator(HANDLE hEngine) :
		WFPEnumerator(hEngine, ::FwpmNetEventCreateEnumHandle, ::FwpmNetEventDestroyEnumHandle, ::FwpmNetEventEnum) { }
};


struct WFPSessionEnumerator : WFPEnumerator<
	decltype(&::FwpmSessionCreateEnumHandle),
	decltype(&::FwpmSessionDestroyEnumHandle),
	decltype(&::FwpmSessionEnum),
	FWPM_SESSION> {

	explicit WFPSessionEnumerator(HANDLE hEngine) : 
		WFPEnumerator(hEngine, ::FwpmSessionCreateEnumHandle, ::FwpmSessionDestroyEnumHandle, ::FwpmSessionEnum) {}
};

struct WFPFilterEnumerator : WFPEnumerator<
	decltype(&::FwpmFilterCreateEnumHandle),
	decltype(&::FwpmFilterDestroyEnumHandle),
	decltype(&::FwpmFilterEnum),
	FWPM_FILTER> {

	explicit WFPFilterEnumerator(HANDLE hEngine) :
		WFPEnumerator(hEngine, ::FwpmFilterCreateEnumHandle, ::FwpmFilterDestroyEnumHandle, ::FwpmFilterEnum) {
	}
};

struct WFPProviderEnumerator : WFPEnumerator<
	decltype(&::FwpmProviderCreateEnumHandle),
	decltype(&::FwpmProviderDestroyEnumHandle),
	decltype(&::FwpmProviderEnum),
	FWPM_PROVIDER> {

	explicit WFPProviderEnumerator(HANDLE hEngine) :
		WFPEnumerator(hEngine, ::FwpmProviderCreateEnumHandle, ::FwpmProviderDestroyEnumHandle, ::FwpmProviderEnum) {
	}
};
