#include "pch.h"
#include "FilterConditionsPage.h"
#include "StringHelper.h"
#include <WFPEngine.h>

CFilterConditionsPage::CFilterConditionsPage(WFPEngine& engine, WFPFilterInfo const& filter) : m_Engine(engine), m_Filter(filter) {
}

CString CFilterConditionsPage::GetColumnText(HWND, int row, int col) const {
    auto& cond = m_Conditions[row];
    switch (col) {
        case 0: return StringHelper::WFPConditionFieldKeyToString(cond.FieldKey);
        case 1: return StringHelper::WFPConditionMatchToString(cond.MatchType);
        case 2: return StringHelper::WFPDataTypeToString(cond.Value.Type);
        case 3: return StringHelper::WFPValueToString(cond.Value, true);
    }
    return CString();
}

void CFilterConditionsPage::OnStateChanged(HWND, int from, int to, UINT oldState, UINT newState) {
    if (newState & LVIS_SELECTED)
        SetDlgItemText(IDC_VALUE, StringHelper::WFPValueToString(m_Conditions[from].Value, true, true));
}

LRESULT CFilterConditionsPage::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
    InitDynamicLayout(false);
    m_List.Attach(GetDlgItem(IDC_LIST));
    m_List.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT);

    auto cm = GetColumnManager(m_List);
    cm->AddColumn(L"Field", 0, 200);
    cm->AddColumn(L"Match", 0, 70);
    cm->AddColumn(L"Type", 0, 80);
    cm->AddColumn(L"Value", 0, 170);
    cm->UpdateColumns();

    auto filter = m_Engine.GetFilterByKey(m_Filter.FilterKey, true);
    ATLASSERT(filter);
    m_Conditions = filter->Conditions;
    m_List.SetItemCount(m_Filter.ConditionCount);

    CWindow edit(GetDlgItem(IDC_VALUE));
    CFontHandle hfont(edit.GetFont());
    LOGFONT lf;
    hfont.GetLogFont(lf);
    wcscpy_s(lf.lfFaceName, L"Consolas");
    lf.lfHeight = lf.lfHeight * 120 / 100;
    CFont font;
    font.CreateFontIndirect(&lf);
    edit.SetFont(font.Detach());

    return 0;
}
