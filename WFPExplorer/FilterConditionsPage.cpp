#include "pch.h"
#include "FilterConditionsPage.h"
#include "StringHelper.h"
#include <WFPEngine.h>

CFilterConditionsPage::CFilterConditionsPage(WFPEngine& engine, FWPM_FILTER* filter) : m_Engine(engine), m_Filter(filter) {
}

CString CFilterConditionsPage::GetColumnText(HWND, int row, int col) const {
    auto& cond = m_Conditions[row];
    switch (col) {
        case 0: return StringHelper::WFPConditionFieldKeyToString(cond.fieldKey);
        case 1: return StringHelper::WFPConditionMatchToString(cond.matchType);
        case 2: return StringHelper::WFPDataTypeToString(cond.conditionValue.type);
        case 3: return StringHelper::WFPConditionValueToString(cond.conditionValue, true, true);
    }
    return CString();
}

void CFilterConditionsPage::OnStateChanged(HWND, int from, int to, UINT oldState, UINT newState) {
    if (newState & LVIS_SELECTED)
        SetDlgItemText(IDC_VALUE, StringHelper::WFPConditionValueToString(m_Conditions[from].conditionValue, true, true));
}

int CFilterConditionsPage::GetRowImage(HWND, int row, int) const {
    return 0;
}

LRESULT CFilterConditionsPage::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
    InitDynamicLayout(false);
    m_List.Attach(GetDlgItem(IDC_LIST));
    m_List.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT);

    auto cm = GetColumnManager(m_List);
    cm->AddColumn(L"Field", 0, 200);
    cm->AddColumn(L"Match", 0, 90);
    cm->AddColumn(L"Type", 0, 80);
    cm->AddColumn(L"Value", 0, 170);
    cm->UpdateColumns();

    m_Conditions.resize(m_Filter->numFilterConditions);
    memcpy(m_Conditions.data(), m_Filter->filterCondition, sizeof(FWPM_FILTER_CONDITION) * m_Filter->numFilterConditions);
    m_List.SetItemCount(m_Filter->numFilterConditions);

    CImageList images;
    images.Create(16, 16, ILC_COLOR32 | ILC_MASK, 2, 2);
    images.AddIcon(AtlLoadIconImage(IDI_FIELD, 0, 16, 16));
    m_List.SetImageList(images, LVSIL_SMALL);

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
