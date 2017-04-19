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
#pragma once
#include "Project64Dialog.h"

class CCheatList;

class CCheatAdd :
    public CProject64Dialog < CCheatAdd >
{
public:
    enum
    {
        WM_EDITCHEAT = WM_USER + 0x120,
    };
    CCheatAdd(CCheatList & ListSection);
    void Create(HWND hParent);

private:
    CCheatAdd(void);                        // Disable default constructor
    CCheatAdd(const CCheatAdd&);            // Disable copy constructor
    CCheatAdd& operator=(const CCheatAdd&); // Disable assignment

    std::string ReadCodeString(bool &validcodes, bool &validoption, bool &nooptions, int &codeformat);
    std::string ReadOptionsString(bool &validcodes, bool &validoptions, bool &nooptions, int &codeformat);
    bool CheatChanged(void);
    void RecordCheatValues(void);
    std::string GetDlgItemStr(int nIDDlgItem);

    BEGIN_MSG_MAP_EX(CEnhancementAdd);
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog);
    MESSAGE_HANDLER(WM_EDITCHEAT, OnEditCheat);
    COMMAND_ID_HANDLER_EX(IDC_ADD, OnAdd);
    COMMAND_ID_HANDLER_EX(IDC_NEWCHEAT, OnNewCheat);
    COMMAND_HANDLER_EX(IDC_CODE_NAME, EN_CHANGE, OnNameChange);
    COMMAND_HANDLER_EX(IDC_CHEAT_CODES, EN_CHANGE, OnCheatCodesChange);
    COMMAND_HANDLER_EX(IDC_CHEAT_OPTIONS, EN_CHANGE, OnCheatOptionsChange);
    END_MSG_MAP()

    LRESULT	OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT	OnEditCheat(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    void OnNameChange(UINT uNotifyCode, int nID, CWindow wndCtl);
    void OnCheatCodesChange(UINT uNotifyCode, int nID, CWindow wndCtl);
    void OnCheatOptionsChange(UINT uNotifyCode, int nID, CWindow wndCtl);
    void OnAdd(UINT uNotifyCode, int nID, CWindow wndCtl);
    void OnNewCheat(UINT uNotifyCode, int nID, CWindow wndCtl);

    //Information about the current cheat we are editing
    std::string m_EditName;
    std::string m_EditCode;
    std::string m_EditOptions;
    std::string m_EditNotes;
    int m_EditCheat;
    CCheatList & m_SelectList;
};

class CCheatList :
    public CProject64Dialog < CCheatList >
{
    enum { IDC_MYTREE = 0x500 };

public:
    enum
    {
        UM_CHANGECODEEXTENSION = WM_USER + 0x121,
    };
    CCheatList(CCheatAdd & AddSection);

    void Create(HWND hParent);
    void Refresh(void);

private:
    CCheatList(void);                         // Disable default constructor
    CCheatList(const CCheatList&);            // Disable copy constructor
    CCheatList& operator=(const CCheatList&); // Disable assignment

    BEGIN_MSG_MAP_EX(CEnhancementList);
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog);
    MESSAGE_HANDLER(WM_DESTROY, OnDestroy);
    MESSAGE_HANDLER(UM_CHANGECODEEXTENSION, OnChangeCodeExtension);
    NOTIFY_HANDLER_EX(IDC_MYTREE, NM_CLICK, OnTreeClick);
    NOTIFY_HANDLER_EX(IDC_MYTREE, NM_RCLICK, OnTreeRightClick);
    NOTIFY_HANDLER_EX(IDC_MYTREE, NM_DBLCLK, OnTreeDBClick);
    NOTIFY_HANDLER_EX(IDC_MYTREE, TVN_SELCHANGEDW, OnTreeSelChange);
    COMMAND_ID_HANDLER_EX(ID_POPUP_DELETE, OnPopupDelete);
    COMMAND_ID_HANDLER_EX(IDC_UNMARK, OnUnMark);
    END_MSG_MAP()

    LRESULT	OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT	OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT	OnChangeCodeExtension(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnTreeClick(LPNMHDR pnmh);
    LRESULT OnTreeRightClick(LPNMHDR pnmh);
    LRESULT OnTreeDBClick(LPNMHDR pnmh);
    LRESULT OnTreeSelChange(LPNMHDR pnmh);
    void OnPopupDelete(UINT uNotifyCode, int nID, CWindow wndCtl);
    void OnUnMark(UINT uNotifyCode, int nID, CWindow wndCtl);

    void AddCodeLayers(int CheatNumber, const std::string &CheatName, HWND hParent, bool CheatActive);
    void ChangeChildrenStatus(HWND hParent, bool Checked);
    void DeleteCheat(int Index);
    void CheckParentStatus(HWND hParent);
    static void MenuSetText(HMENU hMenu, int MenuPos, const wchar_t * Title, const wchar_t * ShortCut);

    CCheatAdd & m_AddSection;
    bool m_DeleteingEntries;
    CTreeViewCtrl m_hCheatTree;
    HWND m_hSelectedItem;
};

class CCheatsUI :
    public CProject64Dialog < CCheatsUI >
{
public:
    CCheatsUI(void);

    bool IsCheatMessage(MSG * msg);
    void SelectCheats(HWND hParent, bool BlockExecution);

private:
    CCheatsUI(const CCheatsUI&);            // Disable copy constructor
    CCheatsUI& operator=(const CCheatsUI&); // Disable assignment

    BEGIN_MSG_MAP_EX(CCheatsUI);
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog);
    COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd);
    COMMAND_ID_HANDLER_EX(IDC_STATE, OnState);
    END_MSG_MAP()

    LRESULT	OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    void OnState(UINT Code, int id, HWND ctl);

    enum Dialog_State { CONTRACTED, EXPANDED } m_DialogState;

    CCheatList m_SelectList;
    CCheatAdd m_AddSection;
    uint32_t m_MinSizeDlg, m_MaxSizeDlg;
};

extern CCheatsUI * g_cheatUI;