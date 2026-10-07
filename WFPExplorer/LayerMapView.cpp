#include "pch.h"
#include "LayerMapView.h"
#include "WFPHelper.h"
#include "StringHelper.h"
#include "AppSettings.h"
#include <WFPEnumerators.h>

using namespace NodeGraphCtrl;

namespace {
	// layout, in graph units
	constexpr float ColumnWidth = 280, NodeWidth = 240, NodeHeight = 46;
	constexpr float RowStep = 110;			// between the rows above the filters (layer, sublayers, ...)
	constexpr float FilterStep = 62, CalloutStep = 66, LegendStep = 58;
	constexpr size_t MaxFilters = 15;		// per sublayer, unless the sublayer is expanded
	constexpr size_t MaxLabel = 30;			// characters that fit in a node's line

	// the providers of Windows Firewall (MPSSVC) filters: FWPM_PROVIDER_MPSSVC_WF, _WSH, _APP_ISOLATION, _EDP, _TENANT_RESTRICTIONS
	const GUID FirewallProviders[] = {
		{ 0xdecc16ca, 0x3f33, 0x4346, { 0xbe, 0x1e, 0x8f, 0xb4, 0xae, 0x0f, 0x3d, 0x62 } },
		{ 0x4b153735, 0x1049, 0x4480, { 0xaa, 0xb4, 0xd1, 0xb9, 0xbd, 0xc0, 0x37, 0x10 } },
		{ 0x3cc2631f, 0x2d5d, 0x43a0, { 0xb1, 0x74, 0x61, 0x48, 0x37, 0xd8, 0x63, 0xa1 } },
		{ 0xa90296f7, 0x46b8, 0x4457, { 0x8f, 0x84, 0xb0, 0x5e, 0x05, 0xd3, 0xc6, 0x22 } },
		{ 0xd0718ff9, 0x44da, 0x4f50, { 0x9d, 0xc2, 0xc9, 0x63, 0xa4, 0x24, 0x76, 0x13 } },
	};

	// fill and border colors that take white text, on a light or a dark background
	const COLORREF Palette[][2] = {
		{ RGB(31, 94, 160), RGB(90, 150, 220) },
		{ RGB(170, 90, 20), RGB(230, 150, 70) },
		{ RGB(35, 120, 55), RGB(90, 180, 110) },
		{ RGB(150, 40, 40), RGB(220, 100, 100) },
		{ RGB(105, 70, 150), RGB(165, 130, 210) },
		{ RGB(20, 120, 130), RGB(80, 190, 200) },
		{ RGB(160, 50, 120), RGB(220, 110, 180) },
		{ RGB(120, 80, 60), RGB(180, 140, 110) },
		{ RGB(110, 110, 25), RGB(190, 190, 80) },
		{ RGB(70, 80, 160), RGB(130, 140, 220) },
		{ RGB(60, 110, 90), RGB(120, 170, 150) },
		{ RGB(130, 60, 70), RGB(190, 120, 130) },
	};

	template<typename TEnum>
	auto EnumerateAll(HANDLE hEngine) {
		TEnum e(hEngine);
		auto all = e.Next(4096);
		for (;;) {
			auto more = e.Next(4096);
			if (more.size() == 0)
				break;
			all.Append(std::move(more));
		}
		return all;
	}

	UINT64 EffectiveWeight(FWPM_FILTER const* f) {
		return f->effectiveWeight.type == FWP_UINT64 && f->effectiveWeight.uint64 ? *f->effectiveWeight.uint64 : 0;
	}

	bool UsesCallout(FWPM_FILTER const* f, GUID const& callout) {
		return (f->action.type & FWP_ACTION_FLAG_CALLOUT) && f->action.calloutKey == callout;
	}

	CString Truncate(CString const& text) {
		return text.GetLength() <= (int)MaxLabel ? text : text.Left((int)MaxLabel - 1) + L"\x2026";
	}

	CString NameOrKey(PCWSTR name, GUID const& key) {
		auto text = name ? StringHelper::ParseMUIString(name) : CString();
		return text.IsEmpty() ? StringHelper::GuidToString(key) : text;
	}

	PCWSTR ActionName(FWP_ACTION_TYPE type) {
		switch (type) {
			case FWP_ACTION_PERMIT: return L"Permit";
			case FWP_ACTION_BLOCK: return L"Block";
			case FWP_ACTION_CALLOUT_TERMINATING: return L"Callout";
			case FWP_ACTION_CALLOUT_INSPECTION: return L"Callout (inspect)";
			case FWP_ACTION_CALLOUT_UNKNOWN: return L"Callout (permit/block)";
			case FWP_ACTION_CONTINUE: return L"Continue";
		}
		return L"Other";
	}

	NodeStyle MakeStyle(COLORREF fill, COLORREF border, COLORREF text = RGB(255, 255, 255)) {
		NodeStyle style;
		style.FillColor = fill;
		style.BorderColor = border;
		style.TextColor = text;
		style.Code = true;		// text scales with the zoom
		return style;
	}

	NodeStyle ActionStyle(FWPM_FILTER const* f) {
		switch (f->action.type) {
			case FWP_ACTION_PERMIT: return MakeStyle(RGB(30, 110, 60), RGB(80, 180, 110));
			case FWP_ACTION_BLOCK: return MakeStyle(RGB(140, 35, 35), RGB(220, 90, 90));
			case FWP_ACTION_CALLOUT_TERMINATING: return MakeStyle(RGB(150, 85, 20), RGB(230, 150, 60));
			case FWP_ACTION_CALLOUT_INSPECTION: return MakeStyle(RGB(110, 90, 30), RGB(200, 170, 80));
			case FWP_ACTION_CALLOUT_UNKNOWN: return MakeStyle(RGB(100, 60, 130), RGB(170, 120, 210));
		}
		return MakeStyle(RGB(70, 70, 70), RGB(140, 140, 140));
	}

	const NodeStyle LayerStyle = MakeStyle(RGB(44, 62, 80), RGB(120, 150, 180));
	const NodeStyle SublayerStyle = MakeStyle(RGB(70, 70, 90), RGB(150, 150, 180));
	const NodeStyle CalloutStyle = MakeStyle(RGB(20, 100, 120), RGB(80, 190, 210));
	const NodeStyle UnusedStyle = MakeStyle(RGB(50, 60, 65), RGB(110, 130, 140));
	const NodeStyle NoProviderStyle = MakeStyle(RGB(70, 70, 70), RGB(140, 140, 140));
	const NodeStyle SummaryStyle = MakeStyle(RGB(55, 55, 55), RGB(110, 110, 110), RGB(210, 210, 210));
	const COLORREF CalloutEdgeColor = RGB(230, 150, 60);

	std::wstring Tooltip(std::wstring text) {
		if (text.size() > 250)
			text.resize(250);
		return text;
	}
}

CLayerMapView::CLayerMapView(IMainFrame* frame, WFPEngine& engine, MapKind kind, bool showList) :
	CFrameView(frame), m_Engine(engine), m_Kind(kind), m_ShowList(showList) {
	auto& settings = AppSettings::Get();
	m_HideFirewall = settings.MapHideFirewall();
	m_ColorByProvider = settings.MapColorByProvider();
}

void CLayerMapView::Refresh() {
	CWaitCursor wait;
	auto hEngine = m_Engine.Handle();
	m_Layers = EnumerateAll<WFPLayerEnumerator>(hEngine);
	m_Sublayers = EnumerateAll<WFPSubLayerEnumerator>(hEngine);
	m_Filters = EnumerateAll<WFPFilterEnumerator>(hEngine);
	m_Callouts = EnumerateAll<WFPCalloutEnumerator>(hEngine);
	m_Providers = EnumerateAll<WFPProviderEnumerator>(hEngine);

	// a provider keeps its color in every graph: the colors go by the providers' names
	m_ProviderNames.clear();
	m_ProviderColors.clear();
	for (auto p : m_Providers) {
		auto name = StringHelper::DisplayName(p->displayData);
		m_ProviderNames[p->providerKey] = name.IsEmpty() ? StringHelper::GuidToString(p->providerKey) : name;
	}
	std::vector<FWPM_PROVIDER*> providers(m_Providers.begin(), m_Providers.end());
	std::ranges::sort(providers, [&](auto a, auto b) { return m_ProviderNames[a->providerKey].CompareNoCase(m_ProviderNames[b->providerKey]) < 0; });
	for (int i = 0; i < (int)providers.size(); i++)
		m_ProviderColors[providers[i]->providerKey] = i;

	// many callouts share a name (e.g. "Windows Firewall: callout"); those get their layer's name as well
	m_CalloutNames.clear();
	std::map<CString, int> nameCount;
	for (auto c : m_Callouts)
		nameCount[NameOrKey(c->displayData.name, c->calloutKey)]++;
	for (auto c : m_Callouts) {
		auto name = NameOrKey(c->displayData.name, c->calloutKey);
		if (nameCount[name] > 1) {
			auto layer = FindLayer(c->applicableLayer);
			auto layerName = layer ? NameOrKey(layer->displayData.name, layer->layerKey) : StringHelper::GuidToString(c->applicableLayer);
			if (layerName.Right(6) == L" Layer")
				layerName = layerName.Left(layerName.GetLength() - 6);
			name += L" (" + layerName + L")";
		}
		m_CalloutNames[c->calloutKey] = name;
	}

	FillList();
}

GUID CLayerMapView::ItemKey(int index) const {
	auto data = m_List.GetItemData(index);
	return m_Kind == MapKind::Layer ? reinterpret_cast<FWPM_LAYER*>(data)->layerKey : reinterpret_cast<FWPM_CALLOUT*>(data)->calloutKey;
}

void CLayerMapView::Select(GUID const& key) {
	for (int i = 0; i < m_List.GetItemCount(); i++) {
		if (ItemKey(i) == key) {
			m_List.SelectItem(i);	// the selection change builds the graph
			return;
		}
	}
	// not in the list (e.g. a layer whose filters are all hidden): shown anyway
	if (key != m_Key) {
		m_Key = key;
		m_Expanded.clear();
		BuildGraph(true);
	}
}

void CLayerMapView::FocusCallout(GUID const& key) {
	m_FocusCallout = key;
	ApplyView();
}

CString CLayerMapView::GetTitle() const {
	if (m_Kind == MapKind::Layer) {
		if (auto layer = FindLayer(m_Key))
			return L"Map: " + NameOrKey(layer->displayData.name, layer->layerKey);
		return L"Layer Map";
	}
	if (auto it = m_CalloutNames.find(m_Key); it != m_CalloutNames.end())
		return L"Callout: " + it->second;
	return L"Callout Map";
}

FWPM_LAYER* CLayerMapView::FindLayer(GUID const& key) const {
	for (auto layer : m_Layers)
		if (layer->layerKey == key)
			return layer;
	return nullptr;
}

FWPM_CALLOUT* CLayerMapView::FindCallout(GUID const& key) const {
	for (auto callout : m_Callouts)
		if (callout->calloutKey == key)
			return callout;
	return nullptr;
}

FWPM_PROVIDER* CLayerMapView::FindProvider(GUID const& key) const {
	for (auto provider : m_Providers)
		if (provider->providerKey == key)
			return provider;
	return nullptr;
}

bool CLayerMapView::IsShown(FWPM_FILTER const* filter) const {
	if (!m_HideFirewall || filter->providerKey == nullptr)
		return true;
	return std::ranges::none_of(FirewallProviders, [&](auto& key) { return key == *filter->providerKey; });
}

CString CLayerMapView::GetProviderName(GUID const* key) const {
	if (key == nullptr)
		return L"";
	if (auto it = m_ProviderNames.find(*key); it != m_ProviderNames.end())
		return it->second;
	return StringHelper::GuidToString(*key);
}

NodeStyle CLayerMapView::ProviderStyle(GUID const* provider) {
	if (provider == nullptr)
		return NoProviderStyle;
	m_UsedProviders.insert(*provider);
	// a provider that wasn't enumerated (deleted since) gets the next color
	auto [it, added] = m_ProviderColors.try_emplace(*provider, (int)m_ProviderColors.size());
	auto& colors = Palette[it->second % _countof(Palette)];
	return MakeStyle(colors[0], colors[1]);
}

NodeStyle CLayerMapView::StyleOf(FWPM_FILTER const* filter) {
	auto style = m_ColorByProvider ? ProviderStyle(filter->providerKey) : ActionStyle(filter);
	if (filter->flags & FWPM_FILTER_FLAG_CLEAR_ACTION_RIGHT) {
		// hard permit/block: lower weight sublayers can't override it
		style.BorderColor = RGB(255, 255, 255);
		style.BorderWidth = 3.0f;
	}
	return style;
}

void CLayerMapView::FillList() {
	struct Item {
		CString Name;
		void* Data;
		GUID Key;
		size_t Filters;
	};
	std::vector<Item> items;
	if (m_Kind == MapKind::Layer) {
		std::map<GUID, size_t, GuidLess> filterCount;
		std::set<GUID, GuidLess> withCallouts;
		for (auto f : m_Filters)
			if (IsShown(f))
				filterCount[f->layerKey]++;
		for (auto c : m_Callouts)
			withCallouts.insert(c->applicableLayer);

		auto hideEmpty = AppSettings::Get().HideEmptyLayers();
		for (auto layer : m_Layers) {
			auto filters = filterCount.contains(layer->layerKey) ? filterCount[layer->layerKey] : 0;
			if (hideEmpty && filters == 0 && !withCallouts.contains(layer->layerKey))
				continue;
			items.push_back({ NameOrKey(layer->displayData.name, layer->layerKey), layer, layer->layerKey, filters });
		}
	}
	else {
		std::map<GUID, size_t, GuidLess> filterCount;
		for (auto f : m_Filters)
			if ((f->action.type & FWP_ACTION_FLAG_CALLOUT) && IsShown(f))
				filterCount[f->action.calloutKey]++;
		for (auto callout : m_Callouts)
			items.push_back({ m_CalloutNames[callout->calloutKey], callout, callout->calloutKey,
				filterCount.contains(callout->calloutKey) ? filterCount[callout->calloutKey] : 0 });
	}
	std::ranges::sort(items, [](auto& a, auto& b) { return a.Name.CompareNoCase(b.Name) < 0; });

	m_List.SetRedraw(FALSE);
	m_List.DeleteAllItems();
	int selected = -1;
	for (int i = 0; i < (int)items.size(); i++) {
		auto& item = items[i];
		m_List.InsertItem(i, item.Name);
		m_List.SetItemText(i, 1, std::to_wstring(item.Filters).c_str());
		m_List.SetItemData(i, reinterpret_cast<DWORD_PTR>(item.Data));
		if (item.Key == m_Key)
			selected = i;
	}
	m_List.SetRedraw(TRUE);

	if (selected < 0 && !items.empty())
		selected = 0;
	m_Key = selected >= 0 ? items[selected].Key : GUID_NULL;
	if (selected >= 0)
		m_List.SelectItem(selected);
	// the layer or callout may be the same one, so rebuild here and not only on the selection change
	BuildGraph(true);
}

NodeId CLayerMapView::AddNode(NodeGraphModel* model, CString const& label, float x, float y, NodeStyle const& style,
	std::wstring tooltip, NodeInfo const& info) {
	auto id = model->AddNode((PCWSTR)label, x, y, NodeWidth, NodeHeight);
	auto node = model->GetNode(id);
	node->Style = style;
	node->Tooltip = Tooltip(std::move(tooltip));
	m_NodeInfo[id] = info;
	return id;
}

Edge* CLayerMapView::AddEdge(NodeGraphModel* model, NodeId from, NodeId to, COLORREF color) {
	auto edge = model->GetEdge(model->AddEdge(from, to));
	edge->Style.Color = color;
	edge->Style.Width = 1.2f;
	return edge;
}

void CLayerMapView::BuildGraph(bool fit) {
	Frame()->SetViewTitle(m_hWnd, GetTitle());
	m_NodeInfo.clear();
	m_UsedProviders.clear();
	auto model = new NodeGraphModel;
	if (m_Kind == MapKind::Layer)
		BuildLayerGraph(model);
	else
		BuildCalloutGraph(model);
	m_Graph.SetModel(model);
	if (fit)
		m_NeedFit = true;
	ApplyView();
}

//
// the sublayers of the filters, in evaluation order (highest weight first), each with its filters in evaluation order
// (highest effective weight first) below it
//
CLayerMapView::Columns CLayerMapView::AddColumns(NodeGraphModel* model, NodeId parent, std::vector<FWPM_FILTER*> filters, float sublayerY) {
	std::map<GUID, FWPM_SUBLAYER*, GuidLess> sublayers;
	for (auto s : m_Sublayers)
		sublayers[s->subLayerKey] = s;

	std::map<GUID, std::vector<FWPM_FILTER*>, GuidLess> groups;
	for (auto f : filters)
		groups[f->subLayerKey].push_back(f);

	struct Column {
		GUID Key;
		FWPM_SUBLAYER* Sublayer;
		std::vector<FWPM_FILTER*>* Filters;
	};
	std::vector<Column> columns;
	for (auto& [key, group] : groups) {
		std::ranges::sort(group, [](auto a, auto b) { return EffectiveWeight(a) > EffectiveWeight(b); });
		columns.push_back({ key, sublayers.contains(key) ? sublayers[key] : nullptr, &group });
	}
	std::ranges::stable_sort(columns, [](auto& a, auto& b) {
		return (a.Sublayer ? a.Sublayer->weight : 0) > (b.Sublayer ? b.Sublayer->weight : 0);
	});

	Columns result;
	result.Count = columns.size();
	result.Filters = filters.size();
	// without columns, what's below starts under the parent
	result.Bottom = columns.empty() ? sublayerY - RowStep + NodeHeight / 2 : sublayerY + NodeHeight / 2;
	const float filtersY = sublayerY + RowStep - 10;

	for (size_t i = 0; i < columns.size(); i++) {
		auto& col = columns[i];
		float x = i * ColumnWidth;
		auto& group = *col.Filters;
		auto sublayerName = col.Sublayer ? NameOrKey(col.Sublayer->displayData.name, col.Key) : StringHelper::GuidToString(col.Key);
		auto weight = col.Sublayer ? col.Sublayer->weight : 0;
		auto provider = col.Sublayer ? col.Sublayer->providerKey : nullptr;
		auto prev = AddNode(model, Truncate(sublayerName) + std::format(L"\nweight 0x{:04X}", weight).c_str(), x, sublayerY,
			m_ColorByProvider ? ProviderStyle(provider) : SublayerStyle,
			std::format(L"{}\nWeight: 0x{:04X}\nProvider: {}\n{} filters shown", (PCWSTR)sublayerName, weight,
				(PCWSTR)GetProviderName(provider), group.size()),
			{ NodeType::Sublayer, col.Sublayer, col.Key });
		AddEdge(model, parent, prev);

		bool expanded = m_Expanded.contains(col.Key);
		size_t shown = expanded ? group.size() : (std::min)(group.size(), MaxFilters);
		float y = filtersY;
		NodeId summary = InvalidNode;
		std::vector<NodeId> filterNodes;
		for (size_t j = 0; j < shown; j++, y += FilterStep) {
			auto f = group[j];
			auto name = NameOrKey(f->displayData.name, f->filterKey);
			bool hard = f->flags & FWPM_FILTER_FLAG_CLEAR_ACTION_RIGHT;
			auto node = AddNode(model, Truncate(name) + L"\n" + ActionName(f->action.type) + (hard ? L" (hard)" : L""), x, y, StyleOf(f),
				std::format(L"{}\nAction: {}{}\nEffective weight: 0x{:016X}\nProvider: {}\nFilter ID: {}, {} conditions",
					(PCWSTR)name, ActionName(f->action.type), hard ? L" (hard)" : L"", EffectiveWeight(f),
					(PCWSTR)GetProviderName(f->providerKey), f->filterId, f->numFilterConditions),
				{ NodeType::Filter, f, f->filterKey });
			AddEdge(model, prev, node);
			filterNodes.push_back(node);
			prev = node;
		}
		if (group.size() > MaxFilters) {
			CString label;
			if (expanded) {
				label = L"Show fewer filters";
			}
			else {
				size_t block = 0, permit = 0, callout = 0;
				for (size_t j = shown; j < group.size(); j++) {
					auto type = group[j]->action.type;
					if (type == FWP_ACTION_BLOCK)
						block++;
					else if (type == FWP_ACTION_PERMIT)
						permit++;
					else if (type & FWP_ACTION_FLAG_CALLOUT)
						callout++;
				}
				label = std::format(L"\x2026 {} more filters\n{} block, {} permit, {} callout", group.size() - shown, block, permit, callout).c_str();
			}
			summary = AddNode(model, label, x, y, SummaryStyle,
				expanded ? L"Double-click to show the first filters only" : L"Double-click to show all the filters of this sublayer",
				{ NodeType::Summary, nullptr, col.Key });
			AddEdge(model, prev, summary);
		}
		result.Bottom = (std::max)(result.Bottom, y - (summary == InvalidNode ? FilterStep : 0) + NodeHeight / 2);

		for (size_t j = 0; j < group.size(); j++) {
			auto f = group[j];
			if ((f->action.type & FWP_ACTION_FLAG_CALLOUT) == 0)
				continue;
			auto& key = f->action.calloutKey;
			if (!result.CalloutUses.contains(key))
				result.CalloutOrder.push_back(key);
			result.CalloutFilters[key]++;
			auto& uses = result.CalloutUses[key];
			if (j < shown)
				uses.push_back({ filterNodes[j], i, filtersY + j * FilterStep });
			else if (std::ranges::none_of(uses, [&](auto& u) { return u.Source == summary; }))
				uses.push_back({ summary, i, y });	// one edge from the summary node is enough
		}
	}
	return result;
}

//
// the providers of the nodes, in their colors, in a column at x
//
void CLayerMapView::AddLegend(NodeGraphModel* model, float x) {
	std::vector<GUID> providers(m_UsedProviders.begin(), m_UsedProviders.end());
	std::ranges::sort(providers, [&](auto& a, auto& b) { return GetProviderName(&a).CompareNoCase(GetProviderName(&b)) < 0; });
	float y = 0;
	for (auto& key : providers) {
		auto name = GetProviderName(&key);
		AddNode(model, L"Provider\n" + Truncate(name), x, y, ProviderStyle(&key),
			std::format(L"{}\n{}\nDouble-click for its properties", (PCWSTR)name, (PCWSTR)StringHelper::GuidToString(key)),
			{ NodeType::Provider, FindProvider(key), key });
		y += LegendStep;
	}
}

void CLayerMapView::BuildLayerGraph(NodeGraphModel* model) {
	auto layer = FindLayer(m_Key);
	if (!layer)
		return;

	std::vector<FWPM_FILTER*> filters;
	for (auto f : m_Filters)
		if (f->layerKey == m_Key && IsShown(f))
			filters.push_back(f);

	auto layerName = NameOrKey(layer->displayData.name, layer->layerKey);
	auto layerNode = AddNode(model, L"", 0, 0, LayerStyle,
		std::format(L"{}\n{}\nLayer ID: {}", (PCWSTR)layerName, (PCWSTR)StringHelper::GuidToString(layer->layerKey), layer->layerId),
		{ NodeType::Layer, layer, layer->layerKey });
	auto cols = AddColumns(model, layerNode, filters, RowStep);
	// over the middle of the columns
	auto node = model->GetNode(layerNode);
	node->Label = (PCWSTR)(Truncate(layerName) + std::format(L"\n{} filters, {} sublayers", cols.Filters, cols.Count).c_str());
	node->X = cols.Count ? (cols.Count - 1) * ColumnWidth / 2 : 0;

	//
	// callouts: below the columns, each under the column that uses it first;
	// callouts of the layer that no filter uses go in a column of their own on the right
	//
	std::vector<std::pair<GUID, size_t>> placed;	// callout, home column
	for (auto& key : cols.CalloutOrder)
		placed.push_back({ key, cols.CalloutUses[key].front().Column });
	for (auto c : m_Callouts)
		if (c->applicableLayer == m_Key && !cols.CalloutUses.contains(c->calloutKey))
			placed.push_back({ c->calloutKey, cols.Count });

	size_t crossEdges = 0;
	for (auto& [key, home] : placed)
		if (cols.CalloutUses.contains(key))
			crossEdges += std::ranges::count_if(cols.CalloutUses[key], [&](auto& u) { return u.Column != home; });
	float busY = cols.Bottom + 30;
	float calloutY = busY + 8.0f * crossEdges + 40 + NodeHeight / 2;

	std::map<size_t, int> perColumn;
	size_t bus = 0, lastColumn = cols.Count ? cols.Count - 1 : 0;
	for (size_t k = 0; k < placed.size(); k++) {
		auto& [key, home] = placed[k];
		lastColumn = (std::max)(lastColumn, home);
		float x = home * ColumnWidth;
		float y = calloutY + perColumn[home]++ * CalloutStep;
		auto callout = FindCallout(key);
		auto name = callout ? NameOrKey(callout->displayData.name, key) : StringHelper::GuidToString(key);
		auto used = cols.CalloutFilters.contains(key) ? cols.CalloutFilters[key] : 0;
		auto provider = callout ? callout->providerKey : nullptr;
		auto calloutNode = AddNode(model, Truncate(name) + (used ? std::format(L"\ncallout, {} filters", used) : std::wstring(L"\ncallout, no filters")).c_str(),
			x, y, m_ColorByProvider ? ProviderStyle(provider) : (used ? CalloutStyle : UnusedStyle),
			std::format(L"{}\nProvider: {}\nUsed by {} filters shown in this layer", (PCWSTR)name, (PCWSTR)GetProviderName(provider), used),
			{ NodeType::Callout, callout, key });

		if (!cols.CalloutUses.contains(key))
			continue;
		// edges run down the gap to the right of a column; edges from another column also cross over below all the columns
		float offset = 6.0f + (k % 5) * 5.0f;
		float homeGapX = x + NodeWidth / 2 + offset;
		for (auto& use : cols.CalloutUses[key]) {
			auto edge = AddEdge(model, use.Source, calloutNode, CalloutEdgeColor);
			float gapX = use.Column * ColumnWidth + NodeWidth / 2 + offset;
			edge->Waypoints.push_back({ gapX, use.Y });
			if (use.Column != home) {
				float y2 = busY + 8.0f * bus++;
				edge->Waypoints.push_back({ gapX, y2 });
				edge->Waypoints.push_back({ homeGapX, y2 });
			}
			edge->Waypoints.push_back({ homeGapX, y });
		}
	}

	if (m_ColorByProvider)
		AddLegend(model, (lastColumn + 1) * ColumnWidth + 40);
}

//
// provider -> callout -> the layer it applies to -> the sublayers of the filters that use the callout, and those filters
//
void CLayerMapView::BuildCalloutGraph(NodeGraphModel* model) {
	auto callout = FindCallout(m_Key);
	if (!callout)
		return;

	std::vector<FWPM_FILTER*> filters;
	for (auto f : m_Filters)
		if (UsesCallout(f, m_Key) && IsShown(f))
			filters.push_back(f);

	std::vector<NodeId> spine;		// the nodes above the columns, to center over them
	float y = 0;
	NodeId calloutParent = InvalidNode;
	if (auto key = callout->providerKey) {
		auto name = GetProviderName(key);
		calloutParent = AddNode(model, L"Provider\n" + Truncate(name), 0, y,
			m_ColorByProvider ? ProviderStyle(key) : MakeStyle(RGB(95, 75, 45), RGB(170, 140, 90)),
			std::format(L"{}\n{}", (PCWSTR)name, (PCWSTR)StringHelper::GuidToString(*key)),
			{ NodeType::Provider, FindProvider(*key), *key });
		spine.push_back(calloutParent);
		y += RowStep;
	}

	auto name = NameOrKey(callout->displayData.name, callout->calloutKey);
	bool registered = callout->flags & FWPM_CALLOUT_FLAG_REGISTERED;
	auto calloutNode = AddNode(model, Truncate(name) + (registered ? L"\ncallout, registered" : L"\ncallout, not registered"), 0, y,
		m_ColorByProvider ? ProviderStyle(callout->providerKey) : CalloutStyle,
		std::format(L"{}\n{}\nProvider: {}\nFlags: {}\nUsed by {} filters shown", (PCWSTR)name,
			(PCWSTR)StringHelper::ParseMUIString(callout->displayData.description), (PCWSTR)GetProviderName(callout->providerKey),
			(PCWSTR)StringHelper::WFPCalloutFlagsToString(callout->flags), filters.size()),
		{ NodeType::Callout, callout, callout->calloutKey });
	spine.push_back(calloutNode);
	if (calloutParent != InvalidNode)
		AddEdge(model, calloutParent, calloutNode);
	y += RowStep;

	auto layer = FindLayer(callout->applicableLayer);
	auto layerName = layer ? NameOrKey(layer->displayData.name, layer->layerKey) : StringHelper::GuidToString(callout->applicableLayer);
	auto layerNode = AddNode(model, Truncate(layerName) + std::format(L"\n{} filters use the callout", filters.size()).c_str(), 0, y, LayerStyle,
		std::format(L"{}\n{}", (PCWSTR)layerName, (PCWSTR)StringHelper::GuidToString(callout->applicableLayer)),
		{ NodeType::Layer, layer, callout->applicableLayer });
	spine.push_back(layerNode);
	AddEdge(model, calloutNode, layerNode);
	y += RowStep;

	Columns cols;
	if (filters.empty()) {
		auto note = AddNode(model, L"No filters use\nthis callout", 0, y, SummaryStyle,
			m_HideFirewall ? L"Windows Firewall filters are hidden" : L"", { NodeType::Note, nullptr, GUID_NULL });
		AddEdge(model, layerNode, note);
	}
	else {
		cols = AddColumns(model, layerNode, filters, y);
	}
	float x = cols.Count ? (cols.Count - 1) * ColumnWidth / 2 : 0;
	for (auto id : spine)
		model->GetNode(id)->X = x;

	if (m_ColorByProvider)
		AddLegend(model, (cols.Count ? cols.Count : 1) * ColumnWidth + 40);
}

//
// fitting and focusing need the graph's size, which a new view doesn't have yet
//
void CLayerMapView::ApplyView() {
	CRect rc;
	m_Graph.GetClientRect(&rc);
	if (rc.IsRectEmpty())
		return;

	if (m_NeedFit) {
		m_NeedFit = false;
		WFPHelper::FitGraph(m_Graph);
	}
	if (m_FocusCallout != GUID_NULL) {
		for (auto& [id, info] : m_NodeInfo) {
			if (info.Type == NodeType::Callout && info.Key == m_FocusCallout) {
				m_Graph.SelectNode(id);
				auto node = m_Graph.GetModel()->GetNode(id);
				m_Graph.CenterOn(node->X, node->Y, (std::max)(m_Graph.GetZoom(), 0.9f));
				break;
			}
		}
		m_FocusCallout = GUID_NULL;
	}
}

void CLayerMapView::UpdateBackground() {
	m_Graph.SetBackgroundColor(WTLHelper::IsDarkMode() ? RGB(38, 38, 38) : RGB(245, 245, 245));
}

void CLayerMapView::ShowOptionsMenu(POINT const& pt) {
	CMenu menu;
	menu.LoadMenu(IDR_CONTEXT);
	CMenuHandle popup = menu.GetSubMenu(6);	// "map options"
	popup.CheckMenuItem(ID_MAP_HIDEFIREWALL, MF_BYCOMMAND | (m_HideFirewall ? MF_CHECKED : MF_UNCHECKED));
	popup.CheckMenuItem(ID_MAP_COLORBYPROVIDER, MF_BYCOMMAND | (m_ColorByProvider ? MF_CHECKED : MF_UNCHECKED));
	BOOL handled;
	switch (auto id = popup.TrackPopupMenu(TPM_RETURNCMD | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd)) {
		case ID_MAP_HIDEFIREWALL:
		case ID_MAP_COLORBYPROVIDER:
			ToggleOption(id);
			break;

		case ID_MAP_FIT:
			m_NeedFit = true;
			ApplyView();
			break;

		case ID_FILE_SAVE:
			OnSave(0, ID_FILE_SAVE, nullptr, handled);
			break;
	}
}

//
// from the graph's menu or the main menu; an option also becomes the default of new maps
//
void CLayerMapView::ToggleOption(UINT id) {
	if (id == ID_MAP_HIDEFIREWALL) {
		m_HideFirewall = !m_HideFirewall;
		AppSettings::Get().MapHideFirewall(m_HideFirewall);
		FillList();		// the filter counts change, and layers may come and go
	}
	else {
		m_ColorByProvider = !m_ColorByProvider;
		AppSettings::Get().MapColorByProvider(m_ColorByProvider);
		BuildGraph(false);
	}
	UpdateUI();
}

void CLayerMapView::UpdateUI() {
	auto& ui = Frame()->UI();
	ui.UIEnable(ID_MAP_HIDEFIREWALL, true);
	ui.UIEnable(ID_MAP_COLORBYPROVIDER, true);
	ui.UISetCheck(ID_MAP_HIDEFIREWALL, m_HideFirewall);
	ui.UISetCheck(ID_MAP_COLORBYPROVIDER, m_ColorByProvider);
}

LRESULT CLayerMapView::OnActivate(UINT, WPARAM active, LPARAM, BOOL&) {
	if (active)
		UpdateUI();
	// 0: the frame still updates the commands every view shares
	return 0;
}

LRESULT CLayerMapView::OnToggleOption(WORD, WORD id, HWND, BOOL&) {
	ToggleOption(id);
	return 0;
}

LRESULT CLayerMapView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	m_hWndClient = m_Splitter.Create(m_hWnd, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
	m_List.Create(m_Splitter, rcDefault, nullptr,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS, 0, IdList);
	// the label tip shows a name that doesn't fit
	m_List.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP);
	// narrow enough for the list's pane, so that the counts show
	m_List.InsertColumn(0, m_Kind == MapKind::Layer ? L"Layer" : L"Callout", LVCFMT_LEFT, 230);
	m_List.InsertColumn(1, L"Filters", LVCFMT_RIGHT, 55);

	m_Graph.Create(m_Splitter, 0, 0, 0, 0, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, NGCS_READONLY);
	m_Graph.SetDlgCtrlID(IdGraph);
	m_Graph.SetMinimapVisible(true);
	UpdateBackground();

	m_Splitter.SetSplitterPanes(m_List, m_Graph);
	m_Splitter.SetSplitterPosPct(22);
	if (!m_ShowList)
		m_Splitter.SetSinglePaneMode(SPLIT_PANE_RIGHT);

	Refresh();
	return 0;
}

LRESULT CLayerMapView::OnSize(UINT, WPARAM wp, LPARAM, BOOL&) {
	if (wp != SIZE_MINIMIZED) {
		UpdateLayout();
		ApplyView();
	}
	return 0;
}

LRESULT CLayerMapView::OnThemeChanged(UINT, WPARAM, LPARAM, BOOL& handled) {
	UpdateBackground();
	handled = FALSE;
	return 0;
}

LRESULT CLayerMapView::OnSelectionChanged(int, LPNMHDR hdr, BOOL&) {
	auto lv = reinterpret_cast<NMLISTVIEW*>(hdr);
	// a right-click selects the item for its menu; the graph stays (the selection is restored after the menu)
	if (::GetKeyState(VK_RBUTTON) < 0)
		return 0;
	if ((lv->uChanged & LVIF_STATE) && (lv->uNewState & LVIS_SELECTED) && (lv->uOldState & LVIS_SELECTED) == 0) {
		auto key = ItemKey(lv->iItem);
		if (key != m_Key) {
			m_Key = key;
			m_Expanded.clear();
			BuildGraph(true);
		}
	}
	return 0;
}

LRESULT CLayerMapView::OnNodeDoubleClick(int, LPNMHDR hdr, BOOL&) {
	auto gn = reinterpret_cast<NODEGRAPHNOTIFY*>(hdr);
	auto it = m_NodeInfo.find(gn->NodeId);
	if (it == m_NodeInfo.end() || (it->second.Data == nullptr && it->second.Type != NodeType::Summary))
		return 0;

	auto& info = it->second;
	switch (info.Type) {
		case NodeType::Layer:
			WFPHelper::ShowLayerProperties(m_Engine, static_cast<FWPM_LAYER*>(info.Data));
			break;

		case NodeType::Sublayer:
			WFPHelper::ShowSublayerProperties(m_Engine, static_cast<FWPM_SUBLAYER*>(info.Data));
			break;

		case NodeType::Filter:
			WFPHelper::ShowFilterProperties(m_Engine, static_cast<FWPM_FILTER*>(info.Data));
			break;

		case NodeType::Callout:
			WFPHelper::ShowCalloutProperties(m_Engine, static_cast<FWPM_CALLOUT*>(info.Data));
			break;

		case NodeType::Provider:
			WFPHelper::ShowProviderProperties(m_Engine, static_cast<FWPM_PROVIDER*>(info.Data));
			break;

		case NodeType::Summary:
			if (!m_Expanded.erase(info.Key))
				m_Expanded.insert(info.Key);
			// the control is still handling the double-click, so the model is replaced later
			PostMessage(WM_REBUILD_GRAPH);
			break;
	}
	return 0;
}

//
// a layer or a callout (a list item or a node) can be shown in a tab of its own; elsewhere in the graph, the map's options
//
LRESULT CLayerMapView::OnContextMenu(UINT, WPARAM, LPARAM lp, BOOL&) {
	CPoint pt(GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
	bool keyboard = lp == -1;
	FWPM_LAYER* layer = nullptr;
	FWPM_CALLOUT* callout = nullptr;
	// wParam is the splitter here (each parent that passes the message on puts itself there), so go by the point or the focus
	HWND hWnd = nullptr;
	if (keyboard) {
		hWnd = ::GetFocus();
	}
	else {
		CRect rc;
		for (HWND h : { m_List.m_hWnd, m_Graph.m_hWnd }) {
			if (::GetWindowRect(h, &rc) && rc.PtInRect(pt))
				hWnd = h;
		}
	}
	if (hWnd == m_List) {
		int index;
		if (keyboard) {
			index = m_List.GetSelectedIndex();
			CRect rc;
			if (index >= 0 && m_List.GetItemRect(index, &rc, LVIR_LABEL))
				pt = rc.CenterPoint();
			m_List.ClientToScreen(&pt);
		}
		else {
			CPoint client(pt);
			m_List.ScreenToClient(&client);
			index = m_List.HitTest(client, nullptr);
		}
		if (index >= 0) {
			auto data = m_List.GetItemData(index);
			if (m_Kind == MapKind::Layer)
				layer = reinterpret_cast<FWPM_LAYER*>(data);
			else
				callout = reinterpret_cast<FWPM_CALLOUT*>(data);
		}
	}
	else if (hWnd == m_Graph) {
		NodeId node;
		if (keyboard) {
			node = m_Graph.GetSelectedNode();
			CRect rc;
			m_Graph.GetClientRect(&rc);
			pt = rc.CenterPoint();
			m_Graph.ClientToScreen(&pt);
		}
		else {
			CPoint client(pt);
			m_Graph.ScreenToClient(&client);
			node = m_Graph.NodeFromPoint(client);
		}
		auto it = m_NodeInfo.find(node);
		if (it != m_NodeInfo.end() && it->second.Type == NodeType::Layer)
			layer = static_cast<FWPM_LAYER*>(it->second.Data);
		else if (it != m_NodeInfo.end() && it->second.Type == NodeType::Callout)
			callout = static_cast<FWPM_CALLOUT*>(it->second.Data);
		else
			ShowOptionsMenu(pt);
	}

	if (layer) {
		switch (WFPHelper::TrackMapMenu(m_hWnd, pt)) {
			case ID_LAYER_SHOWMAP:
				Frame()->ShowLayerMap(layer->layerKey);
				break;
			case ID_EDIT_PROPERTIES:
				WFPHelper::ShowLayerProperties(m_Engine, layer);
				break;
		}
	}
	else if (callout) {
		switch (WFPHelper::TrackMapMenu(m_hWnd, pt, true)) {
			case ID_CALLOUT_SHOWLAYERMAP:
				Frame()->ShowLayerMap(callout->applicableLayer, callout->calloutKey);
				break;
			case ID_CALLOUT_SHOWMAP:
				Frame()->ShowCalloutMap(callout->calloutKey);
				break;
			case ID_EDIT_PROPERTIES:
				WFPHelper::ShowCalloutProperties(m_Engine, callout);
				break;
		}
	}
	// the right-click may have selected another item of the list
	Select(m_Key);
	return 0;
}

LRESULT CLayerMapView::OnRebuildGraph(UINT, WPARAM, LPARAM, BOOL&) {
	BuildGraph(false);
	return 0;
}

LRESULT CLayerMapView::OnRefresh(WORD, WORD, HWND, BOOL&) {
	Refresh();
	return 0;
}

LRESULT CLayerMapView::OnSave(WORD, WORD, HWND, BOOL&) {
	// named after the layer or callout (the title without "Map: " or "Callout: ")
	auto name = GetTitle();
	if (int colon = name.Find(L": "); colon >= 0)
		name = name.Mid(colon + 2);
	WFPHelper::SaveMap(m_hWnd, m_Graph, name);
	return 0;
}
