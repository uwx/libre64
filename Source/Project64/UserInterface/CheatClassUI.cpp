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
#include <Project64-core/Settings/SettingType/SettingsType-Cheats.h>
#include <Project64-core/N64System/CheatClass.h>

extern CCheatsUI * g_cheatUI = NULL;

enum TV_CHECK_STATE { TV_STATE_UNKNOWN, TV_STATE_CLEAR, TV_STATE_CHECKED, TV_STATE_INDETERMINATE };

static std::string GetCheatName(int CheatNo, bool AddExtension);
static bool CheatUsesCodeExtensions(const std::string &LineEntry);
static TV_CHECK_STATE TV_GetCheckState(HWND hwndTreeView, HWND hItem);
static bool TV_SetCheckState(HWND hwndTreeView, HWND hItem, TV_CHECK_STATE state);

class CheatsCodeEx :
    public CProject64Dialog < CCheatList >
{
public:
    CheatsCodeEx(CTreeViewCtrl & hCheatTree, HWND hSelectedItem) :
        m_hCheatTree(hCheatTree),
        m_hSelectedItem(hSelectedItem)
    {
    }

    void Display(HWND hParent)
    {
#ifdef _DEBUG
        m_bModal = true;
#endif
        DialogBoxParamW(GetModuleHandle(NULL), MAKEINTRESOURCEW(IDD_Cheats_CodeEx), hParent, (DLGPROC)WindowProc, (LPARAM)this);
    }

private:
    CheatsCodeEx(void);                           // Disable default constructor
    CheatsCodeEx(const CheatsCodeEx&);            // Disable copy constructor
    CheatsCodeEx& operator=(const CheatsCodeEx&); // Disable assignment

    BEGIN_MSG_MAP_EX(CheatsCodeEx);
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog);
    COMMAND_HANDLER_EX(IDC_CHEAT_LIST, LBN_DBLCLK, OnCheatList);
    COMMAND_ID_HANDLER_EX(IDOK, OnOk);
    COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel);
    END_MSG_MAP()

    LRESULT	OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        //Find the cheat Number of the option being selected
        TVITEM item;
        item.hItem = (HTREEITEM)m_hSelectedItem;
        item.mask = TVIF_PARAM;
        TreeView_GetItem(m_hCheatTree, &item);
        stdstr CheatName = GetCheatName(item.lParam, false);

        //Set up language support for dialog
        SetWindowTextW(m_hWnd, wGS(CHEAT_CODE_EXT_TITLE).c_str());
        SetDlgItemTextW(m_hWnd, IDC_NOTE, wGS(CHEAT_CODE_EXT_TXT).c_str());
        SetDlgItemTextW(m_hWnd, IDOK, wGS(CHEAT_OK).c_str());
        SetDlgItemTextW(m_hWnd, IDCANCEL, wGS(CHEAT_CANCEL).c_str());
        SetDlgItemTextW(m_hWnd, IDC_CHEAT_NAME, CheatName.ToUTF16().c_str());

        //Read through and add all options to the list box
        stdstr Options(g_Settings->LoadStringIndex(Cheat_Options, item.lParam));
        stdstr CurrentExt(g_Settings->LoadStringIndex(Cheat_Extension, item.lParam));
        const char * ReadPos = Options.c_str();
        while (*ReadPos != 0)
        {
            const char * NextComma = strchr(ReadPos, ',');
            int len = NextComma == NULL ? strlen(ReadPos) : NextComma - ReadPos;
            stdstr CheatExt(ReadPos);
            CheatExt.resize(len);
            int index = GetDlgItem(IDC_CHEAT_LIST).SendMessage(LB_ADDSTRING, 0, (LPARAM)CheatExt.c_str());
            if (CheatExt == CurrentExt)
            {
                GetDlgItem(IDC_CHEAT_LIST).SendMessage(LB_SETCURSEL, index, 0);
            }
            //Move to next entry or end
            ReadPos = NextComma ? NextComma + 1 : ReadPos + strlen(ReadPos);
        }
        return true;
    }

    void OnCheatList(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wndCtl*/)
    {
        PostMessage(WM_COMMAND, IDOK, 0);
    }

    void OnCancel(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wndCtl*/)
    {
        //Find the cheat Number of the option being selected
        TVITEM item;
        item.hItem = (HTREEITEM)m_hSelectedItem;
        item.mask = TVIF_PARAM;
        TreeView_GetItem(m_hCheatTree, &item);

        //Get the selected cheat extension
        char CheatExten[300];
        int index = GetDlgItem(IDC_CHEAT_LIST).SendMessage(LB_GETCURSEL, 0, 0);
        if (index < 0) { index = 0; }
        GetDlgItem(IDC_CHEAT_LIST).SendMessage(LB_GETTEXT, index, (LPARAM)CheatExten);

        g_Settings->SaveStringIndex(Cheat_Extension, item.lParam, CheatExten);
        if (g_BaseSystem)
        {
            g_BaseSystem->SetCheatsSlectionChanged(true);
        }
        EndDialog(0);
    }
    void OnOk(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wndCtl*/)
    {
        EndDialog(0);
    }
    HWND m_hSelectedItem;
    CTreeViewCtrl & m_hCheatTree;
};

class CheatsCodeRange :
    public CProject64Dialog < CCheatList >
{
public:
    CheatsCodeRange(CTreeViewCtrl & hCheatTree, HWND hSelectedItem) :
        m_hCheatTree(hCheatTree),
        m_hSelectedItem(hSelectedItem),
        m_Stop(0),
        m_Start(0),
        m_SelStop(0)
    {
    }

    void Display(HWND hParent)
    {
#ifdef _DEBUG
        m_bModal = true;
#endif
        DialogBoxParamW(GetModuleHandle(NULL), MAKEINTRESOURCEW(IDD_Cheats_Range), hParent, (DLGPROC)WindowProc, (LPARAM)this);
    }

private:
    CheatsCodeRange(void);                             // Disable default constructor
    CheatsCodeRange(const CheatsCodeRange&);            // Disable copy constructor
    CheatsCodeRange& operator=(const CheatsCodeRange&); // Disable assignment

    BEGIN_MSG_MAP_EX(CheatsCodeRange);
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog);
    COMMAND_HANDLER_EX(IDC_VALUE, EN_UPDATE, OnValueUpdate);
    COMMAND_ID_HANDLER_EX(IDOK, OnOk);
    COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel);
    END_MSG_MAP()

    uint16_t m_Stop;
    uint16_t m_Start;
    uint16_t m_SelStart;
    uint16_t m_SelStop;

    LRESULT	OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        //Find the cheat Number of the option being selected
        TVITEM item;
        item.hItem = (HTREEITEM)m_hSelectedItem;
        item.mask = TVIF_PARAM;
        TreeView_GetItem(m_hCheatTree, &item);
        stdstr CheatName = GetCheatName(item.lParam, false);
        stdstr RangeNote(g_Settings->LoadStringIndex(Cheat_RangeNotes, item.lParam));
        std::string Range(g_Settings->LoadStringIndex(Cheat_Range, item.lParam));
        stdstr Value(g_Settings->LoadStringIndex(Cheat_Extension, item.lParam));

        //Set up language support for dialog
        SetWindowTextW(m_hWnd, wGS(CHEAT_CODE_EXT_TITLE).c_str());
        SetDlgItemTextW(m_hWnd, IDC_DIGITAL_TEXT, wGS(CHEAT_CHOOSE_VALUE).c_str());
        SetDlgItemTextW(m_hWnd, IDC_VALUE_TEXT, wGS(CHEAT_VALUE).c_str());
        SetDlgItemTextW(m_hWnd, IDC_NOTES_TEXT, wGS(CHEAT_NOTES).c_str());
        SetDlgItemTextW(m_hWnd, IDC_NOTES, RangeNote.ToUTF16().c_str());
        SetDlgItemTextW(m_hWnd, IDC_CHEAT_NAME, CheatName.ToUTF16().c_str());
        SetDlgItemTextW(m_hWnd, IDC_VALUE, Value.ToUTF16().c_str());

        const char * ReadPos = strrchr(Range.c_str(), '-');
        m_Start = (uint16_t)(Range.c_str()[0] == '$' ? strtoul(&Range.c_str()[1], 0, 16) : atol(Range.c_str()));
        m_Stop = ReadPos != NULL ? (uint16_t)(ReadPos[1] == '$' ? strtoul(&ReadPos[2], 0, 16) : atol(&ReadPos[1])) : 0;

        SetDlgItemTextW(m_hWnd, IDC_RANGE, stdstr_f("%ws $%X %ws $%X", GS(CHEAT_FROM), m_Start, GS(CHEAT_TO), m_Stop).ToUTF16().c_str());
        return true;
    }

    void OnValueUpdate(UINT uNotifyCode, int nID, CWindow wndCtl)
    {
        wchar_t szwTmp[20], szwTmp2[20];
        GetDlgItemTextW(m_hWnd, IDC_VALUE, szwTmp, sizeof(szwTmp) / sizeof(szwTmp[0]));
        stdstr Range = stdstr().FromUTF16(szwTmp);
        uint16_t Value = (uint16_t)(Range[0] == L'$' ? strtoul(&Range.c_str()[1], 0, 16) : atol(Range.c_str()));

        if (Value > m_Stop) { Value = m_Stop; }
        if (Value < m_Start) { Value = m_Start; }
        swprintf(szwTmp2, L"$%X", Value);
        if (wcscmp(szwTmp, szwTmp2) != 0)
        {
            /*SetDlgItemTextW(m_hWnd, IDC_VALUE, szwTmp2);
            if (m_SelStop == 0)
            {
                m_SelStop = (uint16_t)wcslen(szwTmp2);
                m_SelStart = m_SelStop;
            }
            SendDlgItemMessageW(m_hWnd, IDC_VALUE, EM_SETSEL, (WPARAM)m_SelStart, (LPARAM)m_SelStop);*/
        }
        else
        {
            /*uint16_t NewSelStart, NewSelStop;
            SendDlgItemMessageW(m_hWnd, IDC_VALUE, EM_GETSEL, (WPARAM)&NewSelStart, (LPARAM)&NewSelStop);
            if (NewSelStart != 0) { m_SelStart = NewSelStart; m_SelStop = NewSelStop; }*/
        }
    }

    void OnCancel(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wndCtl*/)
    {
        EndDialog(0);
    }
    void OnOk(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wndCtl*/)
    {
        EndDialog(0);
    }
    HWND m_hSelectedItem;
    CTreeViewCtrl & m_hCheatTree;
};

/*

int CALLBACK CCheatsUI::CheatsCodeQuantProc(HWND hDlg, uint32_t uMsg, uint32_t wParam, uint32_t lParam)
{
static uint16_t Start, Stop, SelStart, SelStop;

switch (uMsg)
{
case WM_INITDIALOG:
{
}
break;
case WM_COMMAND:
switch (LOWORD(wParam))
{
case IDC_VALUE:
if (HIWORD(wParam) == EN_UPDATE)
{
}
break;
case IDOK:
{
CCheatsUI * _this = (CCheatsUI *)GetProp(hDlg, "Class");

//Find the cheat Number of the option being selected
TVITEM item;
item.hItem = (HTREEITEM)_this->m_hSelectedItem;
item.mask = TVIF_PARAM;
TreeView_GetItem(_this->m_hCheatTree, &item);

//Get the selected cheat extension
TCHAR CheatExten[300], szTmp[10];
uint32_t Value;

GetDlgItemText(hDlg, IDC_VALUE, szTmp, sizeof(szTmp));
Value = szTmp[0] == '$' ? strtol(&szTmp[1], 0, 16) : strtol(&szTmp[0], 0, 16);
if (Value > Stop) { Value = Stop; }
if (Value < Start) { Value = Start; }
sprintf(CheatExten, "$%X", Value);

g_Settings->SaveStringIndex(Cheat_Extension, item.lParam, CheatExten);
if (g_BaseSystem)
{
g_BaseSystem->SetCheatsSlectionChanged(true);
}
}
RemoveProp(hDlg, "Class");
EndDialog(hDlg, 0);
break;
case IDCANCEL:
RemoveProp(hDlg, "Class");
EndDialog(hDlg, 0);
break;
}
default:
return false;
}
return true;
}*/

CCheatsUI::CCheatsUI() :
    m_SelectList(m_AddSection),
    m_AddSection(m_SelectList),
    m_MinSizeDlg(0),
    m_MaxSizeDlg(0)
{
}

void CCheatsUI::SelectCheats(HWND hParent, bool BlockExecution)
{
    if (g_BaseSystem)
    {
        g_BaseSystem->ExternalEvent(SysEvent_PauseCPU_Cheats);
    }
    if (m_hWnd != NULL)
    {
        SetForegroundWindow(m_hWnd);
        return;
    }
    if (hParent == NULL)
    {
        return;
    }
#ifdef _DEBUG
    m_bModal = true;
#endif
    if (BlockExecution)
    {
        DialogBoxParamW(GetModuleHandle(NULL), MAKEINTRESOURCEW(IDD_Cheats_Select), hParent, (DLGPROC)WindowProc, (LPARAM)this);
    }
    else
    {
        CreateDialogParamW(GetModuleHandle(NULL), MAKEINTRESOURCEW(IDD_Cheats_Select), hParent, (DLGPROC)WindowProc, (LPARAM)this);
    }
}

LRESULT	CCheatsUI::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    WINDOWPLACEMENT WndPlac;
    WndPlac.length = sizeof(WndPlac);
    GetWindowPlacement(&WndPlac);

    SetWindowTextW(m_hWnd, wGS(CHEAT_TITLE).c_str());
    m_SelectList.Create(m_hWnd);
    m_SelectList.SetWindowPos(HWND_TOP, 5, 8, 0, 0, SWP_NOSIZE);
    m_SelectList.ShowWindow(SW_SHOW);

    if (!g_Settings->LoadDword(Debugger_Enabled))
    {
        RECT rcList;
        ::GetWindowRect(m_SelectList.GetDlgItem(IDC_CHEATSFRAME), &rcList);
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
        ::GetWindowRect(m_SelectList.GetDlgItem(IDC_CHEATSFRAME), &rcList);
        ::GetWindowRect(m_AddSection.GetDlgItem(IDC_ADDCHEATSFRAME), &rcAdd);
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

    //re-center cheat window
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

LRESULT CCheatsUI::OnCloseCmd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
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
        g_BaseSystem->ExternalEvent(SysEvent_ResumeCPU_Cheats);
    }
    if (g_cheatUI == this)
    {
        delete g_cheatUI;
        g_cheatUI = NULL;
    }
    return true;
}

void CCheatsUI::OnState(UINT /*Code*/, int /*id*/, HWND /*ctl*/)
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

CCheatAdd::CCheatAdd(CCheatList & ListSection) :
    m_SelectList(ListSection),
    m_EditCheat(-1)
{
}

void CCheatAdd::Create(HWND hParent)
{
    CreateDialogParamW(GetModuleHandle(NULL), MAKEINTRESOURCEW(IDD_Cheats_Add), hParent, (DLGPROC)WindowProc, (LPARAM)this);
}

bool CCheatAdd::CheatChanged(void)
{
    bool Changed = false;
    if (m_EditName != GetDlgItemStr(IDC_CODE_NAME) ||
        m_EditCode != GetDlgItemStr(IDC_CHEAT_CODES) ||
        m_EditOptions != GetDlgItemStr(IDC_CHEAT_OPTIONS) ||
        m_EditNotes != GetDlgItemStr(IDC_NOTES))
    {
        Changed = true;
    }
    if (!Changed)
    {
        return false;
    }
    int Result = MessageBoxW(m_hWnd, wGS(CHEAT_CHANGED_MSG).c_str(), wGS(CHEAT_CHANGED_TITLE).c_str(), MB_YESNOCANCEL);
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

std::string CCheatAdd::GetDlgItemStr(int nIDDlgItem)
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

LRESULT	CCheatAdd::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    SetWindowTextW(m_hWnd, wGS(CHEAT_ADDCHEAT_FRAME).c_str());
    SetWindowTextW(GetDlgItem(IDC_NAME), wGS(CHEAT_ADDCHEAT_NAME).c_str());
    SetWindowTextW(GetDlgItem(IDC_CODE), wGS(CHEAT_ADDCHEAT_CODE).c_str());
    SetWindowTextW(GetDlgItem(IDC_LABEL_OPTIONS), wGS(CHEAT_ADDCHEAT_OPT).c_str());
    SetWindowTextW(GetDlgItem(IDC_CODE_DES), wGS(CHEAT_ADDCHEAT_CODEDES).c_str());
    SetWindowTextW(GetDlgItem(IDC_LABEL_OPTIONS_FORMAT), wGS(CHEAT_ADDCHEAT_OPTDES).c_str());
    SetWindowTextW(GetDlgItem(IDC_CHEATNOTES), wGS(CHEAT_ADDCHEAT_NOTES).c_str());
    SetWindowTextW(GetDlgItem(IDC_NEWCHEAT), wGS(CHEAT_ADDCHEAT_NEW).c_str());
    SetWindowTextW(GetDlgItem(IDC_ADD), wGS(CHEAT_ADDCHEAT_ADD).c_str());
    return true;
}

LRESULT	CCheatAdd::OnEditCheat(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    m_EditCheat = wParam;
    if (m_EditCheat < 0)
    {
        return true;
    }

    if (CheatChanged())
    {
        return true;
    }

    stdstr CheatEntryStr = g_Settings->LoadStringIndex(Cheat_Entry, m_EditCheat);
    const char * String = CheatEntryStr.c_str();

    //Set Cheat Name
    int len = strrchr(String, '"') - strchr(String, '"') - 1;
    stdstr CheatName(strchr(String, '"') + 1);
    CheatName.resize(len);
    SetDlgItemText(IDC_CODE_NAME, CheatName.c_str());

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
    SetDlgItemText(IDC_CHEAT_CODES, Buffer.c_str());

    //Add option values to screen
    stdstr CheatOptionStr = g_Settings->LoadStringIndex(Cheat_Options, m_EditCheat);
    ReadPos = strchr(CheatOptionStr.c_str(), '$');
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
    SetDlgItemText(IDC_CHEAT_OPTIONS, Buffer.c_str());

    //Add cheat Notes
    stdstr CheatNotesStr = g_Settings->LoadStringIndex(Cheat_Notes, m_EditCheat);
    SetDlgItemText(IDC_NOTES, CheatNotesStr.c_str());

    SendMessage(WM_COMMAND, MAKELPARAM(IDC_CHEAT_CODES, EN_CHANGE), (LPARAM)(HWND)GetDlgItem(IDC_CHEAT_CODES));
    SetDlgItemTextW(m_hWnd, IDC_ADD, wGS(CHEAT_EDITCHEAT_UPDATE).c_str());

    RecordCheatValues();
    return true;
}

void CCheatAdd::OnNameChange(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wndCtl*/)
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

void CCheatAdd::OnCheatCodesChange(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wndCtl*/)
{
    bool validcodes, validoptions, nooptions;
    int  CodeFormat;
    ReadCodeString(validcodes, validoptions, nooptions, CodeFormat);

    if ((CodeFormat > 0) && !(GetDlgItem(IDC_LABEL_OPTIONS).IsWindowEnabled()))
    {
        GetDlgItem(IDC_LABEL_OPTIONS).EnableWindow(true);
        GetDlgItem(IDC_LABEL_OPTIONS_FORMAT).EnableWindow(true);
        GetDlgItem(IDC_CHEAT_OPTIONS).EnableWindow(true);
    }
    if ((CodeFormat <= 0) && GetDlgItem(IDC_LABEL_OPTIONS).IsWindowEnabled())
    {
        GetDlgItem(IDC_LABEL_OPTIONS).EnableWindow(false);
        GetDlgItem(IDC_LABEL_OPTIONS_FORMAT).EnableWindow(false);
        GetDlgItem(IDC_CHEAT_OPTIONS).EnableWindow(false);
    }

    if (!nooptions)
    {
        ReadOptionsString(validcodes, validoptions, nooptions, CodeFormat);
    }

    bool add = validcodes && (validoptions || nooptions) && SendDlgItemMessageW(m_hWnd, IDC_CODE_NAME, EM_LINELENGTH, 0, 0) > 0;
    GetDlgItem(IDC_ADD).EnableWindow(add);
}

void CCheatAdd::OnCheatOptionsChange(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wndCtl*/)
{
    bool validcodes, validoptions, nooptions;
    int  CodeFormat;
    ReadOptionsString(validcodes, validoptions, nooptions, CodeFormat);

    bool add = validcodes && (validoptions || nooptions) && SendDlgItemMessageW(m_hWnd, IDC_CODE_NAME, EM_LINELENGTH, 0, 0) > 0;
    GetDlgItem(IDC_ADD).EnableWindow(add);
}

void CCheatAdd::OnAdd(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wndCtl*/)
{
    stdstr NewCheatName = GetDlgItemStr(IDC_CODE_NAME);
    bool found = false;
    for (int i = 0; i < CCheats::MaxCheats; i++)
    {
        if (m_EditCheat == i)
        {
            continue;
        }
        stdstr CheatName(GetCheatName(i, false));
        if (CheatName.length() == 0)
        {
            if (m_EditCheat < 0)
            {
                m_EditCheat = i;
                found = true;
            }
            break;
        }
        else if (_stricmp(CheatName.c_str(), NewCheatName.c_str()) == 0)
        {
            g_Notify->DisplayError(GS(MSG_CHEAT_NAME_IN_USE));
            GetDlgItem(IDC_CODE_NAME).SetFocus();
            return;
        }
    }
    if (m_EditCheat < 0 && found)
    {
        g_Notify->DisplayError(GS(MSG_MAX_CHEATS));
        return;
    }

    //Update the entries
    bool validcodes, validoptions, nooptions;
    int  CodeFormat;
    stdstr_f Cheat("\"%s\",%s", NewCheatName.c_str(), ReadCodeString(validcodes, validoptions, nooptions, CodeFormat).c_str());
    stdstr Options = ReadOptionsString(validcodes, validoptions, nooptions, CodeFormat);

    g_Settings->SaveStringIndex(Cheat_Entry, m_EditCheat, Cheat.c_str());
    g_Settings->SaveStringIndex(Cheat_Notes, m_EditCheat, GetDlgItemStr(IDC_NOTES));
    g_Settings->SaveStringIndex(Cheat_Options, m_EditCheat, Options);
    RecordCheatValues();
    CSettingTypeCheats::FlushChanges();
    m_SelectList.Refresh();
}

void CCheatAdd::OnNewCheat(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wndCtl*/)
{
    if (CheatChanged())
    {
        return;
    }
    m_EditCheat = -1;
    SetDlgItemText(IDC_CODE_NAME, "");
    SetDlgItemText(IDC_CHEAT_CODES, "");
    SetDlgItemText(IDC_CHEAT_OPTIONS, "");
    SetDlgItemText(IDC_NOTES, "");
    GetDlgItem(IDC_ADD).EnableWindow(false);
    GetDlgItem(IDC_CHEAT_OPTIONS).EnableWindow(false);
    SetDlgItemTextW(m_hWnd, IDC_ADD, wGS(CHEAT_ADDNEW).c_str());
    RecordCheatValues();
}

void CCheatAdd::RecordCheatValues(void)
{
    m_EditName = GetDlgItemStr(IDC_CODE_NAME);
    m_EditCode = GetDlgItemStr(IDC_CHEAT_CODES);
    m_EditOptions = GetDlgItemStr(IDC_CHEAT_OPTIONS);
    m_EditNotes = GetDlgItemStr(IDC_NOTES);
}

std::string CCheatAdd::ReadCodeString(bool &validcodes, bool &validoption, bool &nooptions, int &codeformat)
{
    validcodes = validoption = nooptions = true;
    codeformat = -1;

    std::string codestring;
    HWND hCheatCodes = GetDlgItem(IDC_CHEAT_CODES);
    uint32_t numlines = SendMessageW(hCheatCodes, EM_GETLINECOUNT, 0, 0);
    if (numlines == 0) { validcodes = false; }

    const wchar_t * formatnormal = L"XXXXXXXX XXXX";
    const wchar_t * formatoptionlb = L"XXXXXXXX XX??";
    const wchar_t * formatoptionw = L"XXXXXXXX ????";
    for (uint32_t linecount = 0; linecount < numlines; linecount++) //read line after line (bypassing limitation GetDlgItemText)
    {
        wchar_t tempformat[128] = { 0 };

        wchar_t str[128] = { 0 };
        *(LPWORD)str = sizeof(str) / sizeof(str[0]);
        uint32_t len = SendMessageW(hCheatCodes, EM_GETLINE, (WPARAM)linecount, (LPARAM)(const wchar_t *)str);
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

std::string CCheatAdd::ReadOptionsString(bool &/*validcodes*/, bool &validoptions, bool &/*nooptions*/, int &codeformat)
{
    validoptions = true;

    HWND hCheatOptions = GetDlgItem(IDC_CHEAT_OPTIONS);
    uint32_t numlines = SendMessageW(hCheatOptions, EM_GETLINECOUNT, 0, 0);
    std::string optionsstring;
    uint32_t numoptions = 0;

    for (uint32_t linecount = 0; linecount < numlines; linecount++) //read line after line (bypassing limitation GetDlgItemText)
    {
        wchar_t str[128] = { 0 };
        *(LPWORD)str = sizeof(str) / sizeof(str[0]);
        uint32_t len = SendMessageW(hCheatOptions, EM_GETLINE, (WPARAM)linecount, (LPARAM)(const wchar_t *)str);
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

bool CCheatsUI::IsCheatMessage(MSG * msg)
{
    if (m_hWnd)
    {
        return IsDialogMessage(msg) != 0;
    }
    return false;
}

CCheatList::CCheatList(CCheatAdd & AddSection) :
    m_DeleteingEntries(false),
    m_hSelectedItem(NULL),
    m_AddSection(AddSection)
{
}

void CCheatList::Create(HWND hParent)
{
    CreateDialogParamW(GetModuleHandle(NULL), MAKEINTRESOURCEW(IDD_Cheats_List), hParent, (DLGPROC)WindowProc, (LPARAM)this);
}

void CCheatList::Refresh(void)
{
    if (m_hWnd == NULL) { return; }

    m_DeleteingEntries = true;
    m_hCheatTree.DeleteAllItems();
    m_DeleteingEntries = false;
    for (int i = 0; i < CCheats::MaxCheats; i++)
    {
        stdstr Name = GetCheatName(i, true);
        if (Name.length() == 0) { break; }

        AddCodeLayers(i, Name, (HWND)TVI_ROOT, g_Settings->LoadBoolIndex(Cheat_Active, i) != 0);
    }
}

LRESULT	CCheatList::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    uint32_t Style;

    SetWindowTextW(GetDlgItem(IDC_CHEATSFRAME), wGS(CHEAT_LIST_FRAME).c_str());
    SetWindowTextW(GetDlgItem(IDC_NOTESFRAME), wGS(CHEAT_NOTES_FRAME).c_str());
    SetWindowTextW(GetDlgItem(IDC_UNMARK), wGS(CHEAT_MARK_NONE).c_str());

    RECT rcList;
    GetDlgItem(IDC_CHEATSFRAME).GetWindowRect(&rcList);

    RECT rcButton;
    GetDlgItem(IDC_UNMARK).GetWindowRect(&rcButton);

    CRect TreeRC(8, 15, rcList.right - rcList.left - 16, rcButton.top - rcList.top - 22);
    m_hCheatTree.Create(m_hWnd, TreeRC, "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | TVS_HASLINES |
        TVS_HASBUTTONS | TVS_LINESATROOT | TVS_DISABLEDRAGDROP | WS_TABSTOP |
        TVS_FULLROWSELECT, WS_EX_CLIENTEDGE, (HMENU)IDC_MYTREE);
    Style = m_hCheatTree.GetWindowLong(GWL_STYLE);
    m_hCheatTree.SetWindowLong(GWL_STYLE, TVS_CHECKBOXES | TVS_SHOWSELALWAYS | Style);

    HIMAGELIST hImageList = ImageList_Create(16, 16, ILC_COLOR | ILC_MASK, 40, 40);
    HBITMAP hBmp = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_TRI_STATE));
    ImageList_AddMasked(hImageList, hBmp, RGB(255, 0, 255));
    DeleteObject(hBmp);
    m_hCheatTree.SetImageList(hImageList, TVSIL_STATE);
    return true;
}

LRESULT	CCheatList::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    if ((HWND)m_hCheatTree != NULL)
    {
        m_hCheatTree.DestroyWindow();
    }
    return true;
}

LRESULT	CCheatList::OnChangeCodeExtension(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
    //Get the selected item
    m_hSelectedItem = (HWND)lParam;

    TVITEM item;
    item.mask = TVIF_PARAM;
    item.hItem = (HTREEITEM)m_hSelectedItem;
    if (!TreeView_GetItem(m_hCheatTree, &item))
    {
        return true;
    }

    //Make sure the selected line can use code extensions
    stdstr LineEntry = g_Settings->LoadStringIndex(Cheat_Entry, item.lParam);
    if (!CheatUsesCodeExtensions(LineEntry))
    {
        return true;
    }

    stdstr Options;
    if (g_Settings->LoadStringIndex(Cheat_Options, item.lParam, Options) && Options.length() > 0)
    {
        CheatsCodeEx(m_hCheatTree, m_hSelectedItem).Display(m_hWnd);
    }
    else
    {
        stdstr Range;
        if (g_Settings->LoadStringIndex(Cheat_Range, item.lParam, Range) && Range.length() > 0)
        {
            CheatsCodeRange(m_hCheatTree, m_hSelectedItem).Display(m_hWnd);
        }
    }

    //Update cheat listing with new extention
    stdstr CheatName(GetCheatName(item.lParam, true));
    char * Cheat = strrchr((char *)CheatName.c_str(), '\\');
    if (Cheat == NULL)
    {
        Cheat = const_cast<char *>(CheatName.c_str());
    }
    else
    {
        Cheat += 1;
    }
    item.mask = TVIF_TEXT;
    item.pszText = Cheat;
    item.cchTextMax = CheatName.length();
    TreeView_SetItem(m_hCheatTree, &item);
    return true;
}

void CCheatList::AddCodeLayers(int CheatNumber, const std::string &CheatName, HWND hParent, bool CheatActive)
{
    TV_INSERTSTRUCT tv;

    //Work out text to add
    char Text[500], Item[500];
    if (CheatName.length() > (sizeof(Text) - 5)) { g_Notify->BreakPoint(__FILE__, __LINE__); }
    strcpy(Text, CheatName.c_str());
    if (strchr(Text, '\\') > 0) { *strchr(Text, '\\') = 0; }

    //See if text is already added
    tv.item.mask = TVIF_TEXT;
    tv.item.pszText = Item;
    tv.item.cchTextMax = sizeof(Item);
    tv.item.hItem = TreeView_GetChild(m_hCheatTree, hParent);
    while (tv.item.hItem)
    {
        TreeView_GetItem(m_hCheatTree, &tv.item);
        if (strcmp(Text, Item) == 0)
        {
            //If already exists then just use existing one
            int State = TV_GetCheckState(m_hCheatTree, (HWND)tv.item.hItem);
            if ((CheatActive && State == TV_STATE_CLEAR) || (!CheatActive && State == TV_STATE_CHECKED))
            {
                TV_SetCheckState(m_hCheatTree, (HWND)tv.item.hItem, TV_STATE_INDETERMINATE);
            }
            size_t StartPos = strlen(Text) + 1;
            stdstr TempCheatName;
            if (StartPos < CheatName.length())
            {
                TempCheatName = CheatName.substr(StartPos);
            }
            AddCodeLayers(CheatNumber, TempCheatName, (HWND)tv.item.hItem, CheatActive);
            return;
        }
        tv.item.hItem = TreeView_GetNextSibling(m_hCheatTree, tv.item.hItem);
    }

    //Add to dialog
    tv.hInsertAfter = TVI_SORT;
    tv.item.mask = TVIF_TEXT | TVIF_PARAM;
    tv.item.pszText = Text;
    tv.item.lParam = CheatNumber;
    tv.hParent = (HTREEITEM)hParent;
    hParent = (HWND)TreeView_InsertItem(m_hCheatTree, &tv);
    TV_SetCheckState(m_hCheatTree, hParent, CheatActive ? TV_STATE_CHECKED : TV_STATE_CLEAR);

    if (strcmp(Text, CheatName.c_str()) == 0) { return; }
    AddCodeLayers(CheatNumber, (stdstr)(CheatName.substr(strlen(Text) + 1)), hParent, CheatActive);
}

void CCheatList::ChangeChildrenStatus(HWND hParent, bool Checked)
{
    HTREEITEM hItem = TreeView_GetChild(m_hCheatTree, hParent);
    if (hItem == NULL)
    {
        if ((HTREEITEM)hParent == TVI_ROOT) { return; }

        TVITEM item;
        item.mask = TVIF_PARAM;
        item.hItem = (HTREEITEM)hParent;
        TreeView_GetItem(m_hCheatTree, &item);

        //if cheat uses a extension and it is not set then do not set it
        if (Checked)
        {
            stdstr LineEntry = g_Settings->LoadStringIndex(Cheat_Entry, item.lParam);
            if (CheatUsesCodeExtensions(LineEntry))
            {
                stdstr CheatExten;
                if (!g_Settings->LoadStringIndex(Cheat_Extension, item.lParam, CheatExten) || CheatExten.empty())
                {
                    return;
                }
            }
        }

        //Save Cheat
        TV_SetCheckState(m_hCheatTree, hParent, Checked ? TV_STATE_CHECKED : TV_STATE_CLEAR);
        g_Settings->SaveBoolIndex(Cheat_Active, item.lParam, Checked);
        return;
    }
    TV_CHECK_STATE state = TV_STATE_UNKNOWN;
    while (hItem != NULL)
    {
        TV_CHECK_STATE ChildState = (TV_CHECK_STATE)TV_GetCheckState(m_hCheatTree, (HWND)hItem);
        if ((ChildState != TV_STATE_CHECKED || !Checked) &&
            (ChildState != TV_STATE_CLEAR || Checked))
        {
            ChangeChildrenStatus((HWND)hItem, Checked);
        }
        ChildState = (TV_CHECK_STATE)TV_GetCheckState(m_hCheatTree, (HWND)hItem);
        if (state == TV_STATE_UNKNOWN) { state = ChildState; }
        if (state != ChildState) { state = TV_STATE_INDETERMINATE; }
        hItem = TreeView_GetNextSibling(m_hCheatTree, hItem);
    }
    if (state != TV_STATE_UNKNOWN)
    {
        TV_SetCheckState(m_hCheatTree, hParent, state);
    }
}

void CCheatList::DeleteCheat(int Index)
{
    for (int CheatNo = Index; CheatNo < CCheats::MaxCheats; CheatNo++)
    {
        stdstr LineEntry = g_Settings->LoadStringIndex(Cheat_Entry, CheatNo + 1);
        if (LineEntry.empty())
        {
            g_Settings->DeleteSettingIndex(Cheat_RangeNotes, CheatNo);
            g_Settings->DeleteSettingIndex(Cheat_Range, CheatNo);
            g_Settings->DeleteSettingIndex(Cheat_Options, CheatNo);
            g_Settings->DeleteSettingIndex(Cheat_Notes, CheatNo);
            g_Settings->DeleteSettingIndex(Cheat_Extension, CheatNo);
            g_Settings->DeleteSettingIndex(Cheat_Entry, CheatNo);
            g_Settings->DeleteSettingIndex(Cheat_Active, CheatNo);
            break;
        }
        stdstr Value;
        if (g_Settings->LoadStringIndex(Cheat_RangeNotes, CheatNo + 1, Value))
        {
            g_Settings->SaveStringIndex(Cheat_RangeNotes, CheatNo, Value);
        }
        else
        {
            g_Settings->DeleteSettingIndex(Cheat_RangeNotes, CheatNo);
        }

        if (g_Settings->LoadStringIndex(Cheat_Range, CheatNo + 1, Value))
        {
            g_Settings->SaveStringIndex(Cheat_Range, CheatNo, Value);
        }
        else
        {
            g_Settings->DeleteSettingIndex(Cheat_Range, CheatNo);
        }

        if (g_Settings->LoadStringIndex(Cheat_Options, CheatNo + 1, Value))
        {
            g_Settings->SaveStringIndex(Cheat_Options, CheatNo, Value);
        }
        else
        {
            g_Settings->DeleteSettingIndex(Cheat_Options, CheatNo);
        }

        if (g_Settings->LoadStringIndex(Cheat_Notes, CheatNo + 1, Value))
        {
            g_Settings->SaveStringIndex(Cheat_Notes, CheatNo, Value);
        }
        else
        {
            g_Settings->DeleteSettingIndex(Cheat_Notes, CheatNo);
        }

        if (g_Settings->LoadStringIndex(Cheat_Extension, CheatNo + 1, Value))
        {
            g_Settings->SaveStringIndex(Cheat_Extension, CheatNo, Value);
        }
        else
        {
            g_Settings->DeleteSettingIndex(Cheat_Extension, CheatNo);
        }

        bool bValue;
        if (g_Settings->LoadBoolIndex(Cheat_Active, CheatNo + 1, bValue))
        {
            g_Settings->SaveBoolIndex(Cheat_Active, CheatNo, bValue);
        }
        else
        {
            g_Settings->DeleteSettingIndex(Cheat_Active, CheatNo);
        }
        g_Settings->SaveStringIndex(Cheat_Entry, CheatNo, LineEntry);
    }
    CSettingTypeCheats::FlushChanges();
}

LRESULT CCheatList::OnTreeClick(LPNMHDR pnmh)
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
        switch (TV_GetCheckState(m_hCheatTree, (HWND)ht.hItem))
        {
        case TV_STATE_CLEAR:
        case TV_STATE_INDETERMINATE:
            //Make sure that the item has a valid code extenstion selected
            if (TreeView_GetChild(m_hCheatTree, ht.hItem) == NULL)
            {
                TVITEM item;
                item.mask = TVIF_PARAM;
                item.hItem = (HTREEITEM)ht.hItem;
                TreeView_GetItem(m_hCheatTree, &item);
                stdstr LineEntry = g_Settings->LoadStringIndex(Cheat_Entry, item.lParam);
                if (CheatUsesCodeExtensions(LineEntry))
                {
                    stdstr CheatExtension;
                    if (!g_Settings->LoadStringIndex(Cheat_Extension, item.lParam, CheatExtension))
                    {
                        SendMessage(UM_CHANGECODEEXTENSION, 0, (LPARAM)ht.hItem);
                        TV_SetCheckState(m_hCheatTree, (HWND)ht.hItem, TV_STATE_CLEAR);
                        break;
                    }
                }
            }
            TV_SetCheckState(m_hCheatTree, (HWND)ht.hItem, TV_STATE_CHECKED);
            ChangeChildrenStatus((HWND)ht.hItem, true);
            CheckParentStatus((HWND)TreeView_GetParent((HWND)m_hCheatTree, (HWND)ht.hItem));
            break;
        case TV_STATE_CHECKED:
            TV_SetCheckState(m_hCheatTree, (HWND)ht.hItem, TV_STATE_CLEAR);
            ChangeChildrenStatus((HWND)ht.hItem, false);
            CheckParentStatus((HWND)TreeView_GetParent((HWND)m_hCheatTree, (HWND)ht.hItem));
            break;
        }
        switch (TV_GetCheckState(m_hCheatTree, (HWND)ht.hItem))
        {
        case TV_STATE_CHECKED: TV_SetCheckState(m_hCheatTree, (HWND)ht.hItem, TV_STATE_INDETERMINATE); break;
        case TV_STATE_CLEAR:   TV_SetCheckState(m_hCheatTree, (HWND)ht.hItem, TV_STATE_CHECKED); break;
        case TV_STATE_INDETERMINATE: TV_SetCheckState(m_hCheatTree, (HWND)ht.hItem, TV_STATE_CLEAR); break;
        }

        if (g_BaseSystem)
        {
            g_BaseSystem->SetCheatsSlectionChanged(true);
        }
    }
    return true;
}

LRESULT CCheatList::OnTreeRightClick(LPNMHDR pnmh)
{
    //Work out what item is selected
    TVHITTESTINFO ht = { 0 };
    uint32_t dwpos = GetMessagePos();

    // include <windowsx.h> and <windows.h> header files
    ht.pt.x = GET_X_LPARAM(dwpos);
    ht.pt.y = GET_Y_LPARAM(dwpos);
    ::MapWindowPoints(HWND_DESKTOP, pnmh->hwndFrom, &ht.pt, 1);

    TreeView_HitTest(pnmh->hwndFrom, &ht);
    m_hSelectedItem = (HWND)ht.hItem;
    if (g_Settings->LoadBool(UserInterface_BasicMode))
    {
        return true;
    }

    //Show Menu
    HMENU hMenu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_CHEAT_MENU));
    HMENU hPopupMenu = GetSubMenu(hMenu, 0);
    POINT Mouse;

    GetCursorPos(&Mouse);

    MenuSetText(hPopupMenu, 0, wGS(CHEAT_ADDNEW).c_str(), NULL);
    MenuSetText(hPopupMenu, 1, wGS(CHEAT_EDIT).c_str(), NULL);
    MenuSetText(hPopupMenu, 3, wGS(CHEAT_DELETE).c_str(), NULL);

    if (m_hSelectedItem != NULL &&
        TreeView_GetChild(m_hCheatTree, m_hSelectedItem) == NULL)
    {
        TrackPopupMenu(hPopupMenu, 0, Mouse.x, Mouse.y, 0, m_hWnd, NULL);
    }
    DestroyMenu(hMenu);
    return true;
}

LRESULT CCheatList::OnTreeDBClick(LPNMHDR pnmh)
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

LRESULT CCheatList::OnTreeSelChange(LPNMHDR /*pnmh*/)
{
    HTREEITEM hItem = TreeView_GetSelection(m_hCheatTree);
    if (TreeView_GetChild(m_hCheatTree, hItem) == NULL)
    {
        TVITEM item;

        item.mask = TVIF_PARAM;
        item.hItem = hItem;
        TreeView_GetItem(m_hCheatTree, &item);

        stdstr Notes(g_Settings->LoadStringIndex(Cheat_Notes, item.lParam));
        SetDlgItemText(IDC_NOTES, Notes.c_str());
        if (m_AddSection)
        {
            SendMessage(m_AddSection, CCheatAdd::WM_EDITCHEAT, item.lParam, 0); //edit cheat
        }
    }
    else
    {
        SetDlgItemText(IDC_NOTES, "");
    }
    return true;
}

void CCheatList::CheckParentStatus(HWND hParent)
{
    TV_CHECK_STATE CurrentState, InitialState;
    HTREEITEM hItem;

    if (!hParent) { return; }
    hItem = TreeView_GetChild(m_hCheatTree, (HTREEITEM)hParent);
    InitialState = (TV_CHECK_STATE)TV_GetCheckState(m_hCheatTree, hParent);
    CurrentState = (TV_CHECK_STATE)TV_GetCheckState(m_hCheatTree, (HWND)hItem);

    while (hItem != NULL)
    {
        if (TV_GetCheckState(m_hCheatTree, (HWND)hItem) != CurrentState)
        {
            CurrentState = TV_STATE_INDETERMINATE;
            break;
        }
        hItem = TreeView_GetNextSibling(m_hCheatTree, hItem);
    }
    TV_SetCheckState(m_hCheatTree, hParent, CurrentState);
    if (InitialState != CurrentState)
    {
        CheckParentStatus((HWND)TreeView_GetParent((HWND)m_hCheatTree, (HTREEITEM)hParent));
    }
}

void CCheatList::MenuSetText(HMENU hMenu, int MenuPos, const wchar_t * Title, const wchar_t * ShortCut)
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

void CCheatList::OnPopupDelete(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wndCtl*/)
{
    TVITEM item;

    int Response = MessageBoxW(GetParent(), wGS(MSG_DEL_SURE).c_str(), wGS(MSG_DEL_TITLE).c_str(), MB_YESNO | MB_ICONQUESTION);
    if (Response != IDYES)
    {
        return;
    }

    //Delete selected cheat
    item.hItem = (HTREEITEM)m_hSelectedItem;
    item.mask = TVIF_PARAM;
    TreeView_GetItem(m_hCheatTree, &item);

    ChangeChildrenStatus((HWND)TVI_ROOT, false);
    DeleteCheat(item.lParam);
    Refresh();
}

void CCheatList::OnUnMark(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wndCtl*/)
{
    ChangeChildrenStatus((HWND)TVI_ROOT, false);
    if (g_BaseSystem != NULL)
    {
        g_BaseSystem->SetCheatsSlectionChanged(true);
    }
}

std::string GetCheatName(int CheatNo, bool AddExtension)
{
    if (CheatNo > CCheats::MaxCheats) { g_Notify->BreakPoint(__FILE__, __LINE__); }
    stdstr LineEntry = g_Settings->LoadStringIndex(Cheat_Entry, CheatNo);
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
    if (AddExtension && CheatUsesCodeExtensions(LineEntry))
    {
        stdstr CheatValue(g_Settings->LoadStringIndex(Cheat_Extension, CheatNo));
        Name.Format("%s (=>%s)", Name.c_str(), CheatValue.c_str());
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

bool CheatUsesCodeExtensions(const std::string &LineEntry)
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