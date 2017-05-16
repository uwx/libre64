/****************************************************************************
*                                                                           *
* Project64 - A Nintendo 64 emulator.                                       *
* http://www.pj64-emu.com/                                                  *
* Copyright (C) 2017 Project64. All rights reserved.                        *
*                                                                           *
* License:                                                                  *
* GNU/GPLv2 http://www.gnu.org/licenses/gpl-2.0.html                        *
*                                                                           *
****************************************************************************/
#include "stdafx.h"
#include "EnhancementUI.h"
#include <Project64-core/Settings/SettingType/SettingsType-Enhancement.h>

enum TV_CHECK_STATE { TV_STATE_UNKNOWN, TV_STATE_CLEAR, TV_STATE_CHECKED, TV_STATE_INDETERMINATE };

static std::string GetEnhancementName(int EnhancementNo, bool AddExtension);
static bool EnhancementUsesCodeExtensions(const std::string &LineEntry);
static TV_CHECK_STATE TV_GetCheckState(HWND hwndTreeView, HWND hItem);
static bool TV_SetCheckState(HWND hwndTreeView, HWND hItem, TV_CHECK_STATE state);

enum
{
    UM_CHANGECODEEXTENSION = WM_USER + 0x121,
};

CEnhancementUI::CEnhancementUI() :
    m_SelectList(m_AddSection),
    m_AddSection(m_SelectList),
    m_MinSizeDlg(0),
    m_MaxSizeDlg(0)
{
}

void CEnhancementUI::Display(HWND hParent)
{
#ifdef _DEBUG
    m_bModal = true;
#endif
    DialogBoxParamW(GetModuleHandle(NULL), MAKEINTRESOURCEW(IDD_Enhancements_Select), hParent, (DLGPROC)WindowProc, (LPARAM)this);
}

LRESULT	CEnhancementUI::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    WINDOWPLACEMENT WndPlac;
    WndPlac.length = sizeof(WndPlac);
    GetWindowPlacement(&WndPlac);

    SetWindowTextW(m_hWnd, wGS(ENHANCEMENT_TITLE).c_str());
    m_SelectList.Create(m_hWnd);
    m_SelectList.SetWindowPos(HWND_TOP, 5, 8, 0, 0, SWP_NOSIZE);
    m_SelectList.ShowWindow(SW_SHOW);

    if (!g_Settings->LoadDword(Debugger_Enabled))
    {
        RECT rcList;
        ::GetWindowRect(m_SelectList.GetDlgItem(IDC_ENHANCEMENTSFRAME), &rcList);
        m_MinSizeDlg = rcList.right - rcList.left + 16;
        m_MaxSizeDlg = m_MinSizeDlg;

        m_DialogState = CONTRACTED;
        WndPlac.rcNormalPosition.right = WndPlac.rcNormalPosition.left + m_MinSizeDlg;
        SetWindowPlacement(&WndPlac);

        GetDlgItem(IDC_STATE).ShowWindow(SW_HIDE);
    }
    else
    {
        RECT & rc = WndPlac.rcNormalPosition;
        m_AddSection.Create(m_hWnd);
        m_AddSection.SetWindowPos(HWND_TOP, (rc.right - rc.left) / 2, 8, 0, 0, SWP_NOSIZE);
        m_AddSection.ShowWindow(SW_HIDE);

        RECT rcList, rcAdd;
        ::GetWindowRect(m_SelectList.GetDlgItem(IDC_ENHANCEMENTSFRAME), &rcList);
        ::GetWindowRect(m_AddSection.GetDlgItem(IDC_ADDENHANCEMENTSFRAME), &rcAdd);
        m_MinSizeDlg = rcList.right - rcList.left + 32;
        m_MaxSizeDlg = rcAdd.right - rcList.left + 32;

        m_DialogState = CONTRACTED;
        WndPlac.rcNormalPosition.right = WndPlac.rcNormalPosition.left + m_MinSizeDlg;
        SetWindowPlacement(&WndPlac);

        GetClientRect(&rc);
        GetDlgItem(IDC_STATE).SetWindowPos(HWND_TOP, (rc.right - rc.left) - 16, 0, 16, rc.bottom - rc.top, 0);
        HANDLE hIcon = LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_RIGHT), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);
        SendDlgItemMessageW(m_hWnd, IDC_STATE, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)(HANDLE)hIcon);
    }

    //re-center enhancement window
    RECT rcDlg, rcParent;
    GetWindowRect(&rcDlg);
    GetParent().GetWindowRect(&rcParent);

    int DlgWidth = rcDlg.right - rcDlg.left;
    int DlgHeight = rcDlg.bottom - rcDlg.top;

    int X = (((rcParent.right - rcParent.left) - DlgWidth) / 2) + rcParent.left;
    int Y = (((rcParent.bottom - rcParent.top) - DlgHeight) / 2) + rcParent.top;

    SetWindowPos(NULL, X, Y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

    m_SelectList.Refresh();
    return true;
}

LRESULT CEnhancementUI::OnCloseCmd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    if ((HWND)m_SelectList != NULL)
    {
        m_SelectList.DestroyWindow();
    }
    if ((HWND)m_AddSection != NULL)
    {
        m_AddSection.DestroyWindow();
    }
    ::SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (LONG_PTR)NULL);
    EndDialog(0);
    m_hWnd = NULL;
    if (g_BaseSystem)
    {
        g_BaseSystem->ExternalEvent(SysEvent_ResumeCPU_Enhancements);
    }
    return true;
}

void CEnhancementUI::OnState(UINT /*Code*/, int /*id*/, HWND /*ctl*/)
{
    WINDOWPLACEMENT WndPlac;
    WndPlac.length = sizeof(WndPlac);
    GetWindowPlacement(&WndPlac);

    if (m_DialogState == CONTRACTED)
    {
        m_DialogState = EXPANDED;
        WndPlac.rcNormalPosition.right = WndPlac.rcNormalPosition.left + m_MaxSizeDlg;
        SetWindowPlacement(&WndPlac);

        RECT clientrect;
        GetClientRect(&clientrect);
        GetDlgItem(IDC_STATE).SetWindowPos(HWND_TOP, (clientrect.right - clientrect.left) - 16, 0, 16, clientrect.bottom - clientrect.top, 0);

        HANDLE hIcon = LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_LEFT), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);
        SendDlgItemMessageW(m_hWnd, IDC_STATE, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)(HANDLE)hIcon);

        m_AddSection.ShowWindow(SW_SHOW);
    }
    else
    {
        m_DialogState = CONTRACTED;
        WndPlac.rcNormalPosition.right = WndPlac.rcNormalPosition.left + m_MinSizeDlg;
        SetWindowPlacement(&WndPlac);

        HANDLE hIcon = LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_RIGHT), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);
        SendDlgItemMessageW(m_hWnd, IDC_STATE, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)(HANDLE)hIcon);

        RECT clientrect;
        GetClientRect(&clientrect);
        GetDlgItem(IDC_STATE).SetWindowPos(HWND_TOP, (clientrect.right - clientrect.left) - 16, 0, 16, clientrect.bottom - clientrect.top, 0);

        m_AddSection.ShowWindow(SW_HIDE);
    }
}

CEnhancementAdd::CEnhancementAdd(CEnhancementList & ListSection) :
    m_SelectList(ListSection)
{
}

void CEnhancementAdd::Create(HWND hParent)
{
    CreateDialogParamW(GetModuleHandle(NULL), MAKEINTRESOURCEW(IDD_Enhancements_Add), hParent, (DLGPROC)WindowProc, (LPARAM)this);
}

bool CEnhancementAdd::EnhancementChanged(void)
{
    bool Changed = false;
    if (m_EditName != GetDlgItemStr(IDC_CODE_NAME) ||
        m_EditCode != GetDlgItemStr(IDC_ENHANCEMENT_CODES) ||
        m_EditOptions != GetDlgItemStr(IDC_ENHANCEMENT_OPTIONS) ||
        m_EditNotes != GetDlgItemStr(IDC_NOTES))
    {
        Changed = true;
    }
    if (!Changed)
    {
        return false;
    }
    int Result = MessageBoxW(m_hWnd, wGS(ENHANCEMENT_CHANGED_MSG).c_str(), wGS(ENHANCEMENT_CHANGED_TITLE).c_str(), MB_YESNOCANCEL);
    if (Result == IDCANCEL)
    {
        return true;
    }
    if (Result == IDYES)
    {
        SendMessageW(m_hWnd, WM_COMMAND, MAKELPARAM(IDC_ADD, 0), (LPARAM)(HWND)GetDlgItem(IDC_ADD));
    }
    return false;
}

std::string CEnhancementAdd::GetDlgItemStr(int nIDDlgItem)
{
    HWND hDlgItem = GetDlgItem(nIDDlgItem);
    int length = SendMessage(hDlgItem, WM_GETTEXTLENGTH, 0, 0);
    if (length == 0)
    {
        return "";
    }

    stdstr Result;
    Result.resize(length + 1);

    ::GetWindowText(hDlgItem, (char *)Result.c_str(), Result.length());
    return Result;
}

LRESULT CEnhancementAdd::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    SetWindowTextW(m_hWnd, wGS(ENHANCEMENT_ADDENHANCEMENT_FRAME).c_str());
    SetWindowTextW(GetDlgItem(IDC_NAME), wGS(ENHANCEMENT_ADDENHANCEMENT_NAME).c_str());
    SetWindowTextW(GetDlgItem(IDC_CODE), wGS(ENHANCEMENT_ADDENHANCEMENT_CODE).c_str());
    SetWindowTextW(GetDlgItem(IDC_LABEL_OPTIONS), wGS(ENHANCEMENT_ADDENHANCEMENT_OPT).c_str());
    SetWindowTextW(GetDlgItem(IDC_CODE_DES), wGS(ENHANCEMENT_ADDENHANCEMENT_CODEDES).c_str());
    SetWindowTextW(GetDlgItem(IDC_LABEL_OPTIONS_FORMAT), wGS(ENHANCEMENT_ADDENHANCEMENT_OPTDES).c_str());
    SetWindowTextW(GetDlgItem(IDC_ENHANCEMENTNOTES), wGS(ENHANCEMENT_ADDENHANCEMENT_NOTES).c_str());
    SetWindowTextW(GetDlgItem(IDC_NEWENHANCEMENT), wGS(ENHANCEMENT_ADDENHANCEMENT_NEW).c_str());
    SetWindowTextW(GetDlgItem(IDC_ADD), wGS(ENHANCEMENT_ADDENHANCEMENT_ADD).c_str());
    return true;
}

LRESULT CEnhancementAdd::OnEditEnhancement(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    m_EditEnhancement = wParam;
    if (m_EditEnhancement < 0)
    {
        return true;
    }

    if (EnhancementChanged())
    {
        return true;
    }

    stdstr EnhancementEntryStr = g_Settings->LoadStringIndex(Enhancement_Entry, m_EditEnhancement);
    const char * String = EnhancementEntryStr.c_str();

    //Set Enhancement Name
    int len = strrchr(String, '"') - strchr(String, '"') - 1;
    stdstr EnhancementName(strchr(String, '"') + 1);
    EnhancementName.resize(len);
    SetDlgItemText(IDC_CODE_NAME, EnhancementName.c_str());

    //Add Gameshark codes to screen
    const char * ReadPos = strrchr(String, '"') + 2;
    stdstr Buffer;
    do
    {
        char * End = strchr((char *)ReadPos, ',');
        if (End)
        {
            Buffer.append(ReadPos, End - ReadPos);
        }
        else
        {
            Buffer.append(ReadPos);
        }

        ReadPos = strchr(ReadPos, ',');
        if (ReadPos != NULL)
        {
            Buffer.append("\r\n");
            ReadPos += 1;
        }
    } while (ReadPos);
    SetDlgItemText(IDC_ENHANCEMENT_CODES, Buffer.c_str());

    //Add option values to screen
    stdstr EnhancementOptionStr = g_Settings->LoadStringIndex(Enhancement_Options, m_EditEnhancement);
    ReadPos = strchr(EnhancementOptionStr.c_str(), '$');
    Buffer.erase();
    if (ReadPos)
    {
        ReadPos += 1;
        do
        {
            char * End = strchr((char *)ReadPos, ',');
            if (End)
            {
                Buffer.append(ReadPos, End - ReadPos);
            }
            else
            {
                Buffer.append(ReadPos);
            }
            ReadPos = strchr(ReadPos, '$');
            if (ReadPos != NULL)
            {
                Buffer.append("\r\n");
                ReadPos += 1;
            }
        } while (ReadPos);
    }
    SetDlgItemText(IDC_ENHANCEMENT_OPTIONS, Buffer.c_str());

    //Add enhancement Notes
    stdstr EnhancementNotesStr = g_Settings->LoadStringIndex(Enhancement_Notes, m_EditEnhancement);
    SetDlgItemText(IDC_NOTES, EnhancementNotesStr.c_str());

    SendMessage(WM_COMMAND, MAKELPARAM(IDC_ENHANCEMENT_CODES, EN_CHANGE), (LPARAM)(HWND)GetDlgItem(IDC_ENHANCEMENT_CODES));
    SetDlgItemTextW(m_hWnd, IDC_ADD, wGS(ENHANCEMENT_EDITENHANCEMENT_UPDATE).c_str());

    RecordEnhancementValues();
    return true;
}

void CEnhancementAdd::OnNameChange(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wndCtl*/)
{
    bool validcodes, validoptions, nooptions;
    int  CodeFormat;
    ReadCodeString(validcodes, validoptions, nooptions, CodeFormat);
    if (!nooptions)
    {
        ReadOptionsString(validcodes, validoptions, nooptions, CodeFormat);
    }

    bool bAdd = validcodes && (validoptions || nooptions) && SendDlgItemMessageW(m_hWnd, IDC_CODE_NAME, EM_LINELENGTH, 0, 0) > 0;
    GetDlgItem(IDC_ADD).EnableWindow(bAdd);
}

void CEnhancementAdd::OnEnhancementCodesChange(UINT /*NotifyCode*/, int /*nID*/, CWindow /*wndCtl*/)
{
    bool validcodes, validoptions, nooptions;
    int  CodeFormat;
    ReadCodeString(validcodes, validoptions, nooptions, CodeFormat);

    if ((CodeFormat > 0) && !GetDlgItem(IDC_LABEL_OPTIONS).IsWindowEnabled())
    {
        GetDlgItem(IDC_LABEL_OPTIONS).EnableWindow(true);
        GetDlgItem(IDC_LABEL_OPTIONS_FORMAT).EnableWindow(true);
        GetDlgItem(IDC_ENHANCEMENT_OPTIONS).EnableWindow(true);
    }
    if ((CodeFormat <= 0) && GetDlgItem(IDC_LABEL_OPTIONS).IsWindowEnabled())
    {
        GetDlgItem(IDC_LABEL_OPTIONS).EnableWindow(false);
        GetDlgItem(IDC_LABEL_OPTIONS_FORMAT).EnableWindow(false);
        GetDlgItem(IDC_ENHANCEMENT_OPTIONS).EnableWindow(false);
    }

    if (!nooptions)
    {
        ReadOptionsString(validcodes, validoptions, nooptions, CodeFormat);
    }

    bool add = validcodes && (validoptions || nooptions) && SendDlgItemMessageW(m_hWnd, IDC_CODE_NAME, EM_LINELENGTH, 0, 0) > 0;
    GetDlgItem(IDC_ADD).EnableWindow(add);
}

void CEnhancementAdd::OnEnhancementOptionsChange(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wndCtl*/)
{
    bool validcodes, validoptions, nooptions;
    int  CodeFormat;
    ReadOptionsString(validcodes, validoptions, nooptions, CodeFormat);

    bool add = validcodes && (validoptions || nooptions) && SendDlgItemMessageW(m_hWnd, IDC_CODE_NAME, EM_LINELENGTH, 0, 0) > 0;
    GetDlgItem(IDC_ADD).EnableWindow(add);
}

void CEnhancementAdd::OnAdd(UINT /*uNotifyCode*/, int /*nIDv*/, CWindow /*wndCtl*/)
{
    stdstr NewEnhancementName = GetDlgItemStr(IDC_CODE_NAME);
    bool found = false;
    for (int i = 0; i < CCheats::MaxEnhancements; i++)
    {
        if (m_EditEnhancement == i)
        {
            continue;
        }
        stdstr EnhancementName(GetEnhancementName(i, false));
        if (EnhancementName.length() == 0)
        {
            if (m_EditEnhancement < 0)
            {
                m_EditEnhancement = i;
                found = true;
            }
            break;
        }
        else if (_stricmp(EnhancementName.c_str(), NewEnhancementName.c_str()) == 0)
        {
            g_Notify->DisplayError(GS(ENHANCEMENT_NAME_IN_USE));
            GetDlgItem(IDC_CODE_NAME).SetFocus();
            return;
        }
    }
    if (m_EditEnhancement < 0 && found)
    {
        g_Notify->DisplayError(GS(ENHANCEMENT_MAX_USED));
        return;
    }

    //Update the entries
    bool validcodes, validoptions, nooptions;
    int  CodeFormat;
    stdstr_f Enhancement("\"%s\",%s", NewEnhancementName.c_str(), ReadCodeString(validcodes, validoptions, nooptions, CodeFormat).c_str());
    stdstr Options = ReadOptionsString(validcodes, validoptions, nooptions, CodeFormat);

    g_Settings->SaveStringIndex(Enhancement_Entry, m_EditEnhancement, Enhancement.c_str());
    g_Settings->SaveStringIndex(Enhancement_Notes, m_EditEnhancement, GetDlgItemStr(IDC_NOTES));
    g_Settings->SaveStringIndex(Enhancement_Options, m_EditEnhancement, Options);
    RecordEnhancementValues();
    CSettingTypeEnhancement::FlushChanges();
    m_SelectList.Refresh();
}

void CEnhancementAdd::OnNewEnhancement(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wndCtl*/)
{
    if (EnhancementChanged())
    {
        return;
    }
    m_EditEnhancement = -1;
    SetDlgItemText(IDC_CODE_NAME, "");
    SetDlgItemText(IDC_ENHANCEMENT_CODES, "");
    SetDlgItemText(IDC_ENHANCEMENT_OPTIONS, "");
    SetDlgItemText(IDC_NOTES, "");
    GetDlgItem(IDC_ADD).EnableWindow(false);
    GetDlgItem(IDC_ENHANCEMENT_OPTIONS).EnableWindow(false);
    SetDlgItemTextW(m_hWnd, IDC_ADD, wGS(ENHANCEMENT_ADDNEW).c_str());
    RecordEnhancementValues();
}

void CEnhancementAdd::RecordEnhancementValues(void)
{
    m_EditName = GetDlgItemStr(IDC_CODE_NAME);
    m_EditCode = GetDlgItemStr(IDC_ENHANCEMENT_CODES);
    m_EditOptions = GetDlgItemStr(IDC_ENHANCEMENT_OPTIONS);
    m_EditNotes = GetDlgItemStr(IDC_NOTES);
}

std::string CEnhancementAdd::ReadCodeString(bool &validcodes, bool &validoption, bool &nooptions, int &codeformat)
{
    validcodes = validoption = nooptions = true;
    codeformat = -1;

    std::string codestring;
    HWND hEnhancementCodes = GetDlgItem(IDC_ENHANCEMENT_CODES);
    uint32_t numlines = SendMessageW(hEnhancementCodes, EM_GETLINECOUNT, 0, 0);
    if (numlines == 0) { validcodes = false; }

    const wchar_t * formatnormal = L"XXXXXXXX XXXX";
    const wchar_t * formatoptionlb = L"XXXXXXXX XX??";
    const wchar_t * formatoptionw = L"XXXXXXXX ????";
    for (uint32_t linecount = 0; linecount < numlines; linecount++) //read line after line (bypassing limitation GetDlgItemText)
    {
        wchar_t tempformat[128] = { 0 };

        wchar_t str[128] = { 0 };
        *(LPWORD)str = sizeof(str) / sizeof(str[0]);
        uint32_t len = SendMessageW(hEnhancementCodes, EM_GETLINE, (WPARAM)linecount, (LPARAM)(const wchar_t *)str);
        str[len] = 0;

        if (len <= 0) { continue; }

        for (uint32_t i = 0; i < 128; i++)
        {
            if (iswxdigit(str[i]) != 0)
            {
                tempformat[i] = L'X';
            }
            if ((str[i] == ' ') || (str[i] == '?'))
            {
                tempformat[i] = str[i];
            }
            if (str[i] == 0) { break; }
        }
        if (wcscmp(tempformat, formatnormal) == 0)
        {
            codestring += codestring.empty() ? "" : ",";
            codestring += stdstr().FromUTF16(str);
            if (codeformat < 0)
            {
                codeformat = 0;
            }
        }
        else if (wcscmp(tempformat, formatoptionlb) == 0)
        {
            if (codeformat != 2)
            {
                codestring += codestring.empty() ? "" : ",";
                codestring += stdstr().FromUTF16(str);
                codeformat = 1;
                nooptions = false;
                validoption = false;
            }
            else
            {
                validcodes = false;
            }
        }
        else if (wcscmp(tempformat, formatoptionw) == 0)
        {
            if (codeformat != 1)
            {
                codestring += codestring.empty() ? "" : ",";
                codestring += stdstr().FromUTF16(str);
                codeformat = 2;
                nooptions = false;
                validoption = false;
            }
            else
            {
                validcodes = false;
            }
        }
        else
        {
            validcodes = false;
        }
    }
    if (codestring.length() == 0)
    {
        validcodes = false;
    }
    return codestring;
}

std::string CEnhancementAdd::ReadOptionsString(bool &/*validcodes*/, bool &validoptions, bool &/*nooptions*/, int &codeformat)
{
    validoptions = true;

    HWND hEnhancementOptions = GetDlgItem(IDC_ENHANCEMENT_OPTIONS);
    uint32_t numlines = SendMessageW(hEnhancementOptions, EM_GETLINECOUNT, 0, 0);
    std::string optionsstring;
    uint32_t numoptions = 0;

    for (uint32_t linecount = 0; linecount < numlines; linecount++) //read line after line (bypassing limitation GetDlgItemText)
    {
        wchar_t str[128] = { 0 };
        *(LPWORD)str = sizeof(str) / sizeof(str[0]);
        uint32_t len = SendMessageW(hEnhancementOptions, EM_GETLINE, (WPARAM)linecount, (LPARAM)(const wchar_t *)str);
        str[len] = 0;

        if (len <= 0) { continue; }
        switch (codeformat)
        {
        case 1: //option = lower byte
            if (len >= 2)
            {
                for (int i = 0; i < 2; i++)
                {
                    if (iswxdigit(str[i]) == 0)
                    {
                        validoptions = false;
                        break;
                    }
                }

                if ((str[2] != L' ') && (len > 2))
                {
                    validoptions = false;
                    break;
                }

                for (int i = 0; i < 2; i++)
                {
                    str[i] = (char)towupper(str[i]);
                }

                optionsstring += optionsstring.empty() ? "" : ",";
                optionsstring += "$";
                optionsstring += stdstr().FromUTF16(str);
                numoptions++;
            }
            else
            {
                validoptions = false;
                break;
            }
            break;

        case 2: //option = word
            if (len >= 4)
            {
                for (uint32_t i = 0; i < 4; i++)
                {
                    if (!isxdigit(str[i]))
                    {
                        validoptions = false;
                        break;
                    }
                }

                if (str[4] != ' ' && (len > 4))
                {
                    validoptions = false;
                    break;
                }

                for (uint32_t i = 0; i < 4; i++)
                {
                    str[i] = (char)towupper(str[i]);
                }

                optionsstring += optionsstring.empty() ? "" : ",";
                optionsstring += "$";
                optionsstring += stdstr().FromUTF16(str);
                numoptions++;
            }
            else
            {
                validoptions = false;
                break;
            }
            break;
        default:
            break;
        }
    }

    if (numoptions < 1)
    {
        validoptions = false;
    }
    return optionsstring;
}

CEnhancementList::CEnhancementList(CEnhancementAdd & AddSection) :
    m_DeleteingEntries(false),
    m_hSelectedItem(NULL),
    m_AddSection(AddSection)
{
}

void CEnhancementList::Create(HWND hParent)
{
    CreateDialogParamW(GetModuleHandle(NULL), MAKEINTRESOURCEW(IDD_Enhancements_List), hParent, (DLGPROC)WindowProc, (LPARAM)this);
}

void CEnhancementList::Refresh(void)
{
    if (m_hWnd == NULL) { return; }

    m_DeleteingEntries = true;
    m_hEnhancementTree.DeleteAllItems();
    m_DeleteingEntries = false;
    for (int i = 0; i < CCheats::MaxEnhancements; i++)
    {
        stdstr Name = GetEnhancementName(i, true);
        if (Name.length() == 0) { break; }

        AddCodeLayers(i, Name, (HWND)TVI_ROOT, g_Settings->LoadBoolIndex(Enhancement_Active, i) != 0);
    }
}

LRESULT CEnhancementList::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    uint32_t Style;

    SetWindowTextW(GetDlgItem(IDC_ENHANCEMENTSFRAME), wGS(ENHANCEMENT_LIST_FRAME).c_str());
    SetWindowTextW(GetDlgItem(IDC_NOTESFRAME), wGS(ENHANCEMENT_NOTES_FRAME).c_str());
    SetWindowTextW(GetDlgItem(IDC_UNMARK), wGS(ENHANCEMENT_MARK_NONE).c_str());

    RECT rcList;
    GetDlgItem(IDC_ENHANCEMENTSFRAME).GetWindowRect(&rcList);

    RECT rcButton;
    GetDlgItem(IDC_UNMARK).GetWindowRect(&rcButton);

    CRect TreeRC(8, 15, rcList.right - rcList.left - 16, rcButton.top - rcList.top - 22);
    m_hEnhancementTree.Create(m_hWnd, TreeRC, "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | TVS_HASLINES |
        TVS_HASBUTTONS | TVS_LINESATROOT | TVS_DISABLEDRAGDROP | WS_TABSTOP |
        TVS_FULLROWSELECT, WS_EX_CLIENTEDGE, (HMENU)IDC_MYTREE);
    Style = m_hEnhancementTree.GetWindowLong(GWL_STYLE);
    m_hEnhancementTree.SetWindowLong(GWL_STYLE, TVS_CHECKBOXES | TVS_SHOWSELALWAYS | Style);

    HIMAGELIST hImageList = ImageList_Create(16, 16, ILC_COLOR | ILC_MASK, 40, 40);
    HBITMAP hBmp = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_TRI_STATE));
    ImageList_AddMasked(hImageList, hBmp, RGB(255, 0, 255));
    DeleteObject(hBmp);
    m_hEnhancementTree.SetImageList(hImageList, TVSIL_STATE);
    return true;
}

LRESULT	CEnhancementList::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    if ((HWND)m_hEnhancementTree != NULL)
    {
        m_hEnhancementTree.DestroyWindow();
    }
    return true;
}

void CEnhancementList::AddCodeLayers(int EnhancementNumber, const std::string &EnhancementName, HWND hParent, bool EnhancementActive)
{
    TV_INSERTSTRUCT tv;

    //Work out text to add
    char Text[500], Item[500];
    if (EnhancementName.length() > (sizeof(Text) - 5)) { g_Notify->BreakPoint(__FILE__, __LINE__); }
    strcpy(Text, EnhancementName.c_str());
    if (strchr(Text, '\\') > 0) { *strchr(Text, '\\') = 0; }

    //See if text is already added
    tv.item.mask = TVIF_TEXT;
    tv.item.pszText = Item;
    tv.item.cchTextMax = sizeof(Item);
    tv.item.hItem = TreeView_GetChild(m_hEnhancementTree, hParent);
    while (tv.item.hItem)
    {
        TreeView_GetItem(m_hEnhancementTree, &tv.item);
        if (strcmp(Text, Item) == 0)
        {
            //If already exists then just use existing one
            int State = TV_GetCheckState(m_hEnhancementTree, (HWND)tv.item.hItem);
            if ((EnhancementActive && State == TV_STATE_CLEAR) || (!EnhancementActive && State == TV_STATE_CHECKED))
            {
                TV_SetCheckState(m_hEnhancementTree, (HWND)tv.item.hItem, TV_STATE_INDETERMINATE);
            }
            size_t StartPos = strlen(Text) + 1;
            stdstr TempEnhancementName;
            if (StartPos < EnhancementName.length())
            {
                TempEnhancementName = EnhancementName.substr(StartPos);
            }
            AddCodeLayers(EnhancementNumber, TempEnhancementName, (HWND)tv.item.hItem, EnhancementActive);
            return;
        }
        tv.item.hItem = TreeView_GetNextSibling(m_hEnhancementTree, tv.item.hItem);
    }

    //Add to dialog
    tv.hInsertAfter = TVI_SORT;
    tv.item.mask = TVIF_TEXT | TVIF_PARAM;
    tv.item.pszText = Text;
    tv.item.lParam = EnhancementNumber;
    tv.hParent = (HTREEITEM)hParent;
    hParent = (HWND)TreeView_InsertItem(m_hEnhancementTree, &tv);
    TV_SetCheckState(m_hEnhancementTree, hParent, EnhancementActive ? TV_STATE_CHECKED : TV_STATE_CLEAR);

    if (strcmp(Text, EnhancementName.c_str()) == 0) { return; }
    AddCodeLayers(EnhancementNumber, (stdstr)(EnhancementName.substr(strlen(Text) + 1)), hParent, EnhancementActive);
}

void CEnhancementList::DeleteEnhancement(int Index)
{
    for (int EnhancementNo = Index; EnhancementNo < CCheats::MaxEnhancements; EnhancementNo++)
    {
        stdstr LineEntry = g_Settings->LoadStringIndex(Enhancement_Entry, EnhancementNo + 1);
        if (LineEntry.empty())
        {
            g_Settings->DeleteSettingIndex(Enhancement_RangeNotes, EnhancementNo);
            g_Settings->DeleteSettingIndex(Enhancement_Range, EnhancementNo);
            g_Settings->DeleteSettingIndex(Enhancement_Options, EnhancementNo);
            g_Settings->DeleteSettingIndex(Enhancement_Notes, EnhancementNo);
            g_Settings->DeleteSettingIndex(Enhancement_Extension, EnhancementNo);
            g_Settings->DeleteSettingIndex(Enhancement_Entry, EnhancementNo);
            g_Settings->DeleteSettingIndex(Enhancement_Active, EnhancementNo);
            break;
        }
        stdstr Value;
        if (g_Settings->LoadStringIndex(Enhancement_RangeNotes, EnhancementNo + 1, Value))
        {
            g_Settings->SaveStringIndex(Enhancement_RangeNotes, EnhancementNo, Value);
        }
        else
        {
            g_Settings->DeleteSettingIndex(Enhancement_RangeNotes, EnhancementNo);
        }

        if (g_Settings->LoadStringIndex(Enhancement_Range, EnhancementNo + 1, Value))
        {
            g_Settings->SaveStringIndex(Enhancement_Range, EnhancementNo, Value);
        }
        else
        {
            g_Settings->DeleteSettingIndex(Enhancement_Range, EnhancementNo);
        }

        if (g_Settings->LoadStringIndex(Enhancement_Options, EnhancementNo + 1, Value))
        {
            g_Settings->SaveStringIndex(Enhancement_Options, EnhancementNo, Value);
        }
        else
        {
            g_Settings->DeleteSettingIndex(Enhancement_Options, EnhancementNo);
        }

        if (g_Settings->LoadStringIndex(Enhancement_Notes, EnhancementNo + 1, Value))
        {
            g_Settings->SaveStringIndex(Enhancement_Notes, EnhancementNo, Value);
        }
        else
        {
            g_Settings->DeleteSettingIndex(Enhancement_Notes, EnhancementNo);
        }

        if (g_Settings->LoadStringIndex(Enhancement_Extension, EnhancementNo + 1, Value))
        {
            g_Settings->SaveStringIndex(Enhancement_Extension, EnhancementNo, Value);
        }
        else
        {
            g_Settings->DeleteSettingIndex(Enhancement_Extension, EnhancementNo);
        }

        bool bValue;
        if (g_Settings->LoadBoolIndex(Enhancement_Active, EnhancementNo + 1, bValue))
        {
            g_Settings->SaveBoolIndex(Enhancement_Active, EnhancementNo, bValue);
        }
        else
        {
            g_Settings->DeleteSettingIndex(Enhancement_Active, EnhancementNo);
        }
        g_Settings->SaveStringIndex(Enhancement_Entry, EnhancementNo, LineEntry);
    }
    //CSettingTypeEnhancements::FlushChanges();
}

LRESULT CEnhancementList::OnTreeClick(LPNMHDR pnmh)
{
    TVHITTESTINFO ht = { 0 };
    uint32_t dwpos = GetMessagePos();

    // include <windowsx.h> and <windows.h> header files
    ht.pt.x = GET_X_LPARAM(dwpos);
    ht.pt.y = GET_Y_LPARAM(dwpos);
    ::MapWindowPoints(HWND_DESKTOP, pnmh->hwndFrom, &ht.pt, 1);

    TreeView_HitTest(pnmh->hwndFrom, &ht);

    if (TVHT_ONITEMSTATEICON & ht.flags)
    {
        switch (TV_GetCheckState(m_hEnhancementTree, (HWND)ht.hItem))
        {
        case TV_STATE_CLEAR:
        case TV_STATE_INDETERMINATE:
            //Make sure that the item has a valid code extenstion selected
            if (TreeView_GetChild(m_hEnhancementTree, ht.hItem) == NULL)
            {
                TVITEM item;
                item.mask = TVIF_PARAM;
                item.hItem = (HTREEITEM)ht.hItem;
                TreeView_GetItem(m_hEnhancementTree, &item);
                stdstr LineEntry = g_Settings->LoadStringIndex(Enhancement_Entry, item.lParam);
                if (EnhancementUsesCodeExtensions(LineEntry))
                {
                    stdstr EnhancementExtension;
                    if (!g_Settings->LoadStringIndex(Enhancement_Extension, item.lParam, EnhancementExtension))
                    {
                        SendMessage(UM_CHANGECODEEXTENSION, 0, (LPARAM)ht.hItem);
                        TV_SetCheckState(m_hEnhancementTree, (HWND)ht.hItem, TV_STATE_CLEAR);
                        break;
                    }
                }
            }
            TV_SetCheckState(m_hEnhancementTree, (HWND)ht.hItem, TV_STATE_CHECKED);
            ChangeChildrenStatus((HWND)ht.hItem, true);
            CheckParentStatus((HWND)TreeView_GetParent((HWND)m_hEnhancementTree, (HWND)ht.hItem));
            break;
        case TV_STATE_CHECKED:
            TV_SetCheckState(m_hEnhancementTree, (HWND)ht.hItem, TV_STATE_CLEAR);
            ChangeChildrenStatus((HWND)ht.hItem, false);
            CheckParentStatus((HWND)TreeView_GetParent((HWND)m_hEnhancementTree, (HWND)ht.hItem));
            break;
        }
        switch (TV_GetCheckState(m_hEnhancementTree, (HWND)ht.hItem))
        {
        case TV_STATE_CHECKED: TV_SetCheckState(m_hEnhancementTree, (HWND)ht.hItem, TV_STATE_INDETERMINATE); break;
        case TV_STATE_CLEAR:   TV_SetCheckState(m_hEnhancementTree, (HWND)ht.hItem, TV_STATE_CHECKED); break;
        case TV_STATE_INDETERMINATE: TV_SetCheckState(m_hEnhancementTree, (HWND)ht.hItem, TV_STATE_CLEAR); break;
        }

        if (g_BaseSystem)
        {
            g_BaseSystem->SetCheatsSlectionChanged(true);
        }
    }
    return true;
}

LRESULT CEnhancementList::OnTreeDBClick(LPNMHDR pnmh)
{
    TVHITTESTINFO ht = { 0 };
    uint32_t dwpos = GetMessagePos();

    // include <windowsx.h> and <windows.h> header files
    ht.pt.x = GET_X_LPARAM(dwpos);
    ht.pt.y = GET_Y_LPARAM(dwpos);
    ::MapWindowPoints(HWND_DESKTOP, pnmh->hwndFrom, &ht.pt, 1);

    TreeView_HitTest(pnmh->hwndFrom, &ht);

    if (TVHT_ONITEMLABEL & ht.flags)
    {
        PostMessage(UM_CHANGECODEEXTENSION, 0, (LPARAM)ht.hItem);
    }
    return true;
}

LRESULT CEnhancementList::OnTreeSelChange(LPNMHDR /*pnmh*/)
{
    HTREEITEM hItem = TreeView_GetSelection(m_hEnhancementTree);
    if (TreeView_GetChild(m_hEnhancementTree, hItem) == NULL)
    {
        TVITEM item;

        item.mask = TVIF_PARAM;
        item.hItem = hItem;
        TreeView_GetItem(m_hEnhancementTree, &item);

        stdstr Notes(g_Settings->LoadStringIndex(Enhancement_Notes, item.lParam));
        SetDlgItemText(IDC_NOTES, Notes.c_str());
        if (m_AddSection)
        {
            SendMessage(m_AddSection, CEnhancementAdd::WM_EDITENHANCEMENT, item.lParam, 0); //edit enhancement
        }
    }
    else
    {
        SetDlgItemText(IDC_NOTES, "");
    }
    return true;
}

void CEnhancementList::CheckParentStatus(HWND hParent)
{
    TV_CHECK_STATE CurrentState, InitialState;
    HTREEITEM hItem;

    if (!hParent) { return; }
    hItem = TreeView_GetChild(m_hEnhancementTree, (HTREEITEM)hParent);
    InitialState = (TV_CHECK_STATE)TV_GetCheckState(m_hEnhancementTree, hParent);
    CurrentState = (TV_CHECK_STATE)TV_GetCheckState(m_hEnhancementTree, (HWND)hItem);

    while (hItem != NULL)
    {
        if (TV_GetCheckState(m_hEnhancementTree, (HWND)hItem) != CurrentState)
        {
            CurrentState = TV_STATE_INDETERMINATE;
            break;
        }
        hItem = TreeView_GetNextSibling(m_hEnhancementTree, hItem);
    }
    TV_SetCheckState(m_hEnhancementTree, hParent, CurrentState);
    if (InitialState != CurrentState)
    {
        CheckParentStatus((HWND)TreeView_GetParent((HWND)m_hEnhancementTree, (HTREEITEM)hParent));
    }
}

void CEnhancementList::MenuSetText(HMENU hMenu, int MenuPos, const wchar_t * Title, const wchar_t * ShortCut)
{
    MENUITEMINFOW MenuInfo;
    wchar_t String[256];

    if (Title == NULL || wcslen(Title) == 0) { return; }

    memset(&MenuInfo, 0, sizeof(MENUITEMINFO));
    MenuInfo.cbSize = sizeof(MENUITEMINFO);
    MenuInfo.fMask = MIIM_TYPE;
    MenuInfo.fType = MFT_STRING;
    MenuInfo.fState = MFS_ENABLED;
    MenuInfo.dwTypeData = String;
    MenuInfo.cch = 256;

    GetMenuItemInfoW(hMenu, MenuPos, true, &MenuInfo);
    wcscpy(String, Title);
    if (wcschr(String, '\t') != NULL) { *(wcschr(String, '\t')) = '\0'; }
    if (ShortCut) { _swprintf(String, L"%s\t%s", String, ShortCut); }
    SetMenuItemInfoW(hMenu, MenuPos, true, &MenuInfo);
}

void CEnhancementList::ChangeChildrenStatus(HWND hParent, bool Checked)
{
    HTREEITEM hItem = TreeView_GetChild(m_hEnhancementTree, hParent);
    if (hItem == NULL)
    {
        if ((HTREEITEM)hParent == TVI_ROOT) { return; }

        TVITEM item;
        item.mask = TVIF_PARAM;
        item.hItem = (HTREEITEM)hParent;
        TreeView_GetItem(m_hEnhancementTree, &item);

        //if enhancement uses a extension and it is not set then do not set it
        if (Checked)
        {
            stdstr LineEntry = g_Settings->LoadStringIndex(Enhancement_Entry, item.lParam);
            if (EnhancementUsesCodeExtensions(LineEntry))
            {
                stdstr EnhancementExten;
                if (!g_Settings->LoadStringIndex(Enhancement_Extension, item.lParam, EnhancementExten) || EnhancementExten.empty())
                {
                    return;
                }
            }
        }

        //Save Enhancement
        TV_SetCheckState(m_hEnhancementTree, hParent, Checked ? TV_STATE_CHECKED : TV_STATE_CLEAR);
        g_Settings->SaveBoolIndex(Enhancement_Active, item.lParam, Checked);
        return;
    }
    TV_CHECK_STATE state = TV_STATE_UNKNOWN;
    while (hItem != NULL)
    {
        TV_CHECK_STATE ChildState = (TV_CHECK_STATE)TV_GetCheckState(m_hEnhancementTree, (HWND)hItem);
        if ((ChildState != TV_STATE_CHECKED || !Checked) &&
            (ChildState != TV_STATE_CLEAR || Checked))
        {
            ChangeChildrenStatus((HWND)hItem, Checked);
        }
        ChildState = (TV_CHECK_STATE)TV_GetCheckState(m_hEnhancementTree, (HWND)hItem);
        if (state == TV_STATE_UNKNOWN) { state = ChildState; }
        if (state != ChildState) { state = TV_STATE_INDETERMINATE; }
        hItem = TreeView_GetNextSibling(m_hEnhancementTree, hItem);
    }
    if (state != TV_STATE_UNKNOWN)
    {
        TV_SetCheckState(m_hEnhancementTree, hParent, state);
    }
}

void CEnhancementList::OnPopupDelete(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wndCtl*/)
{
    TVITEM item;

    int Response = MessageBoxW(GetParent(), wGS(MSG_DEL_SURE).c_str(), wGS(MSG_DEL_TITLE).c_str(), MB_YESNO | MB_ICONQUESTION);
    if (Response != IDYES)
    {
        return;
    }

    //Delete selected enhancement
    item.hItem = (HTREEITEM)m_hSelectedItem;
    item.mask = TVIF_PARAM;
    TreeView_GetItem(m_hEnhancementTree, &item);

    ChangeChildrenStatus((HWND)TVI_ROOT, false);
    DeleteEnhancement(item.lParam);
    Refresh();
}

void CEnhancementList::OnUnMark(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wndCtl*/)
{
    ChangeChildrenStatus((HWND)TVI_ROOT, false);
    if (g_BaseSystem != NULL)
    {
        g_BaseSystem->SetCheatsSlectionChanged(true);
    }
}

std::string GetEnhancementName(int EnhancementNo, bool AddExtension)
{
    if (EnhancementNo > CCheats::MaxEnhancements) { g_Notify->BreakPoint(__FILE__, __LINE__); }
    stdstr LineEntry = g_Settings->LoadStringIndex(Enhancement_Entry, EnhancementNo);
    if (LineEntry.length() == 0) { return LineEntry; }

    //Find the start and end of the name which is surrounded in ""
    int StartOfName = LineEntry.find("\"");
    if (StartOfName == -1) { return stdstr(""); }
    int EndOfName = LineEntry.find("\"", StartOfName + 1);
    if (EndOfName == -1) { return stdstr(""); }

    stdstr Name = LineEntry.substr(StartOfName + 1, EndOfName - StartOfName - 1);
    const char * CodeString = &(LineEntry.c_str())[EndOfName + 2];
    if (!CCheats::IsValid16BitCode(CodeString))
    {
        Name.Format("*** %s", Name.c_str());
        Name.Replace("\\", "\\*** ");
    }
    if (AddExtension && EnhancementUsesCodeExtensions(LineEntry))
    {
        stdstr EnhancementValue(g_Settings->LoadStringIndex(Enhancement_Extension, EnhancementNo));
        Name.Format("%s (=>%s)", Name.c_str(), EnhancementValue.c_str());
    }
    return Name;
}

TV_CHECK_STATE TV_GetCheckState(HWND hwndTreeView, HWND hItem)
{
    TVITEM tvItem;

    // Prepare to receive the desired information.
    tvItem.mask = TVIF_HANDLE | TVIF_STATE;
    tvItem.hItem = (HTREEITEM)hItem;
    tvItem.stateMask = TVIS_STATEIMAGEMASK;

    // Request the information.
    TreeView_GetItem(hwndTreeView, &tvItem);

    // Return zero if it's not checked, or nonzero otherwise.
    switch (tvItem.state >> 12) {
    case 1: return TV_STATE_CHECKED;
    case 2: return TV_STATE_CLEAR;
    case 3: return TV_STATE_INDETERMINATE;
    }
    return (TV_CHECK_STATE)((int)(tvItem.state >> 12) - 1);
}

bool TV_SetCheckState(HWND hwndTreeView, HWND hItem, TV_CHECK_STATE state)
{
    TVITEM tvItem;

    tvItem.mask = TVIF_HANDLE | TVIF_STATE;
    tvItem.hItem = (HTREEITEM)hItem;
    tvItem.stateMask = TVIS_STATEIMAGEMASK;

    /*Image 1 in the tree-view check box image list is the
    unchecked box. Image 2 is the checked box.*/

    switch (state)
    {
    case TV_STATE_CHECKED: tvItem.state = INDEXTOSTATEIMAGEMASK(1); break;
    case TV_STATE_CLEAR: tvItem.state = INDEXTOSTATEIMAGEMASK(2); break;
    case TV_STATE_INDETERMINATE: tvItem.state = INDEXTOSTATEIMAGEMASK(3); break;
    default: tvItem.state = INDEXTOSTATEIMAGEMASK(0); break;
    }
    return TreeView_SetItem(hwndTreeView, &tvItem) != 0;
}

bool EnhancementUsesCodeExtensions(const std::string &LineEntry)
{
    //Find the start and end of the name which is surronded in ""
    if (LineEntry.length() == 0) { return false; }
    int StartOfName = LineEntry.find("\"");
    if (StartOfName == -1) { return false; }
    int EndOfName = LineEntry.find("\"", StartOfName + 1);
    if (EndOfName == -1) { return false; }

    //Read through the gameshark entries till you find a ??
    const char *ReadPos = &(LineEntry.c_str())[EndOfName + 2];
    bool CodeExtension = false;

    for (int i = 0; i < CCheats::MaxGSEntries && CodeExtension == false; i++)
    {
        if (strchr(ReadPos, ' ') == NULL) { break; }
        ReadPos = strchr(ReadPos, ' ') + 1;
        if (ReadPos[0] == '?' && ReadPos[1] == '?') { CodeExtension = true; }
        if (ReadPos[2] == '?' && ReadPos[3] == '?') { CodeExtension = true; }
        if (strchr(ReadPos, ',') == NULL) { continue; }
        ReadPos = strchr(ReadPos, ',') + 1;
    }
    return CodeExtension;
}