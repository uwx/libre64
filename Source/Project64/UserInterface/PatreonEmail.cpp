#include "stdafx.h"
#include "PatreonEmail.h"
#include <Project64/Patreon.h>
#include <Project64-core/Settings/SettingType/SettingsType-Application.h>
#include "resource.h"

LRESULT CPatreonEmail::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    SetWindowTextW(m_hWnd, wGS(MSG_SUPPORT_PATREON_EMAIL_TITLE).c_str());
    SetWindowTextW(GetDlgItem(IDC_CODE), stdstr(UISettingsLoadStringVal(SupportWindows_PatreonEmail)).ToUTF16().c_str() );
    SetWindowTextW(GetDlgItem(IDOK), wGS(MSG_SUPPORT_OK).c_str());
    SetWindowTextW(GetDlgItem(IDCANCEL), wGS(MSG_SUPPORT_CANCEL).c_str());
    SetWindowTextW(GetDlgItem(IDC_DESCRIPTION), wGS(MSG_SUPPORT_PATREON_EMAIL_DESC).c_str());
    return TRUE;
}

LRESULT CPatreonEmail::OnColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    HDC hdcStatic = (HDC)wParam;
    SetTextColor(hdcStatic, RGB(0, 0, 0));
    SetBkMode(hdcStatic, TRANSPARENT);
    return (LONG)(LRESULT)((HBRUSH)GetStockObject(NULL_BRUSH));
}

LRESULT CPatreonEmail::OnEraseBackground(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    static HPEN outline = CreatePen(PS_SOLID, 1, 0x00FFFFFF);
    static HBRUSH fill = CreateSolidBrush(0x00FFFFFF);
    SelectObject((HDC)wParam, outline);
    SelectObject((HDC)wParam, fill);

    RECT rect;
    GetClientRect(&rect);

    Rectangle((HDC)wParam, rect.left, rect.top, rect.right, rect.bottom);
    return TRUE;
}

LRESULT CPatreonEmail::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    EndDialog(wID);
    return TRUE;
}

LRESULT CPatreonEmail::OnOkCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    char email[255];
    if (!GetDlgItemText(IDC_CODE, email,sizeof(email)))
    {
        email[0] = '\0';
    }
    UISettingsSaveString(SupportWindows_PatreonEmail, email);
    CSettingTypeApplication::Flush();
    EndDialog(wID);
    return TRUE;
}