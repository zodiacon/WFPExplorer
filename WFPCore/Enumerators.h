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

struct WFPCalloutEnumerator : WFPEnumerator<
	decltype(&::FwpmCalloutCreateEnumHandle),
	decltype(&::FwpmCalloutDestroyEnumHandle),
	decltype(&::FwpmCalloutEnum),
	FWPM_CALLOUT> {

	explicit WFPCalloutEnumerator(HANDLE hEngine) :
		WFPEnumerator(hEngine, ::FwpmCalloutCreateEnumHandle, ::FwpmCalloutDestroyEnumHandle, ::FwpmCalloutEnum) {
	}
};

struct WFPLayerEnumerator : WFPEnumerator<
	decltype(&::FwpmLayerCreateEnumHandle),
	decltype(&::FwpmLayerDestroyEnumHandle),
	decltype(&::FwpmLayerEnum),
	FWPM_LAYER> {

	explicit WFPLayerEnumerator(HANDLE hEngine) :
		WFPEnumerator(hEngine, ::FwpmLayerCreateEnumHandle, ::FwpmLayerDestroyEnumHandle, ::FwpmLayerEnum) {
	}
};

struct WFPSubLayerEnumerator : WFPEnumerator<
	decltype(&::FwpmSubLayerCreateEnumHandle),
	decltype(&::FwpmSubLayerDestroyEnumHandle),
	decltype(&::FwpmSubLayerEnum),
	FWPM_SUBLAYER> {

	explicit WFPSubLayerEnumerator(HANDLE hEngine) :
		WFPEnumerator(hEngine, ::FwpmSubLayerCreateEnumHandle, ::FwpmSubLayerDestroyEnumHandle, ::FwpmSubLayerEnum) {
	}
};

struct WFPProviderContextEnumerator : WFPEnumerator<
	decltype(&::FwpmProviderContextCreateEnumHandle),
	decltype(&::FwpmProviderContextDestroyEnumHandle),
	decltype(&::FwpmProviderContextEnum),
	FWPM_PROVIDER_CONTEXT> {

	explicit WFPProviderContextEnumerator(HANDLE hEngine) :
		WFPEnumerator(hEngine, ::FwpmProviderContextCreateEnumHandle, ::FwpmProviderContextDestroyEnumHandle, ::FwpmProviderContextEnum) {
	}
};

struct WFPConnectionEnumerator : WFPEnumerator<
	decltype(&::FwpmConnectionCreateEnumHandle),
	decltype(&::FwpmConnectionDestroyEnumHandle),
	decltype(&::FwpmConnectionEnum),
	FWPM_CONNECTION> {

	explicit WFPConnectionEnumerator(HANDLE hEngine) :
		WFPEnumerator(hEngine, ::FwpmConnectionCreateEnumHandle, ::FwpmConnectionDestroyEnumHandle, ::FwpmConnectionEnum) {
	}
};

