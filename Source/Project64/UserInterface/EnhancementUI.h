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

class CEnhancementList;

class CEnhancementAdd :
    public CProject64Dialog < CEnhancementAdd >
{
public:
    enum
    {
        WM_EDITENHANCEMENT = WM_USER + 0x120,
    };
    CEnhancementAdd(CEnhancementList & ListSection);
    void Create(HWND hParent);

private:
    CEnhancementAdd(void);                        // Disable default constructor
    CEnhancementAdd(const CEnhancementAdd&);            // Disable copy constructor
    CEnhancementAdd& operator=(const CEnhancementAdd&); // Disable assignment

    std::string ReadCodeString(bool &validcodes, bool &validoption, bool &nooptions, int &codeformat);
    std::string ReadOptionsString(bool &validcodes, bool &validoptions, bool &nooptions, int &codeformat);
    bool EnhancementChanged(void);
    void RecordEnhancementValues(void);
    std::string GetDlgItemStr(int nIDDlgItem);

    BEGIN_MSG_MAP_EX(CEnhancementAdd);
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog);
    MESSAGE_HANDLER(WM_EDITENHANCEMENT, OnEditEnhancement);
    COMMAND_ID_HANDLER_EX(IDC_ADD, OnAdd);
    COMMAND_ID_HANDLER_EX(IDC_NEWENHANCEMENT, OnNewEnhancement);
    COMMAND_HANDLER_EX(IDC_CODE_NAME, EN_CHANGE, OnNameChange);
    COMMAND_HANDLER_EX(IDC_ENHANCEMENT_CODES, EN_CHANGE, OnEnhancementCodesChange);
    COMMAND_HANDLER_EX(IDC_ENHANCEMENT_OPTIONS, EN_CHANGE, OnEnhancementOptionsChange);
    END_MSG_MAP()

    LRESULT	OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT	OnEditEnhancement(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    void OnNameChange(UINT uNotifyCode, int nID, CWindow wndCtl);
    void OnEnhancementCodesChange(UINT uNotifyCode, int nID, CWindow wndCtl);
    void OnEnhancementOptionsChange(UINT uNotifyCode, int nID, CWindow wndCtl);
    void OnAdd(UINT uNotifyCode, int nID, CWindow wndCtl);
    void OnNewEnhancement(UINT uNotifyCode, int nID, CWindow wndCtl);

    //Information about the current enhancement we are editing
    std::string m_EditName;
    std::string m_EditCode;
    std::string m_EditOptions;
    std::string m_EditNotes;
    int32_t m_EditEnhancement;
    CEnhancementList & m_SelectList;
};

class CEnhancementList :
    public CProject64Dialog < CEnhancementList >
{
    enum
    {
        IDC_MYTREE = 0x500
    };

public:
    CEnhancementList(CEnhancementAdd & AddSection);

    void Create(HWND hParent);
    void Refresh(void);

private:
    CEnhancementList(void);                         // Disable default constructor
    CEnhancementList(const CEnhancementList&);            // Disable copy constructor
    CEnhancementList& operator=(const CEnhancementList&); // Disable assignment

    BEGIN_MSG_MAP_EX(CEnhancementList);
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog);
    MESSAGE_HANDLER(WM_DESTROY, OnDestroy);
    NOTIFY_HANDLER_EX(IDC_MYTREE, NM_CLICK, OnTreeClick);
    NOTIFY_HANDLER_EX(IDC_MYTREE, NM_DBLCLK, OnTreeDBClick);
    NOTIFY_HANDLER_EX(IDC_MYTREE, TVN_SELCHANGEDW, OnTreeSelChange);
    COMMAND_ID_HANDLER_EX(ID_POPUP_DELETE, OnPopupDelete);
    COMMAND_ID_HANDLER_EX(IDC_UNMARK, OnUnMark);
    END_MSG_MAP()

    LRESULT	OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT	OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnTreeClick(LPNMHDR pnmh);
    LRESULT OnTreeDBClick(LPNMHDR pnmh);
    LRESULT OnTreeSelChange(LPNMHDR pnmh);
    void OnPopupDelete(UINT uNotifyCode, int nID, CWindow wndCtl);
    void OnUnMark(UINT uNotifyCode, int nID, CWindow wndCtl);

    void AddCodeLayers(int EnhancementNumber, const std::string &EnhancementName, HWND hParent, bool EnhancementActive);
    void ChangeChildrenStatus(HWND hParent, bool Checked);
    void DeleteEnhancement(int Index);
    void CheckParentStatus(HWND hParent);
    static void MenuSetText(HMENU hMenu, int MenuPos, const wchar_t * Title, const wchar_t * ShortCut);

    CEnhancementAdd & m_AddSection;
    bool m_DeleteingEntries;
    CTreeViewCtrl m_hEnhancementTree;
    HWND m_hSelectedItem;
};

class CEnhancementUI :
    public CProject64Dialog < CEnhancementUI >
{
public:
    CEnhancementUI();

    void Display(HWND hParent);

private:
    CEnhancementUI(const CEnhancementUI&);            // Disable copy constructor
    CEnhancementUI& operator=(const CEnhancementUI&); // Disable assignment

    BEGIN_MSG_MAP_EX(CEnhancementUI);
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog);
    COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd);
    COMMAND_ID_HANDLER_EX(IDC_STATE, OnState);
    END_MSG_MAP()

    LRESULT	OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    void OnState(UINT Code, int id, HWND ctl);

    enum Dialog_State { CONTRACTED, EXPANDED } m_DialogState;

    CEnhancementList m_SelectList;
    CEnhancementAdd m_AddSection;
    uint32_t m_MinSizeDlg, m_MaxSizeDlg;
};
