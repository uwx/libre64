/****************************************************************************
*                                                                           *
* Project64 - A Nintendo 64 emulator.                                      *
* http://www.pj64-emu.com/                                                  *
* Copyright (C) 2012 Project64. All rights reserved.                        *
*                                                                           *
* License:                                                                  *
* GNU/GPLv2 http://www.gnu.org/licenses/gpl-2.0.html                        *
*                                                                           *
****************************************************************************/
#include "stdafx.h"
#include "SettingsType-Enhancement.h"

CIniFile * CSettingTypeEnhancement::m_EnhancementIniFile = NULL;
std::string * CSettingTypeEnhancement::m_SectionIdent = NULL;

CSettingTypeEnhancement::CSettingTypeEnhancement(const char * PostFix) :
    m_PostFix(PostFix)
{
}

CSettingTypeEnhancement::~CSettingTypeEnhancement(void)
{
}

void CSettingTypeEnhancement::Initialize(void)
{
    WriteTrace(TraceAppInit, TraceDebug, "Start");
    m_EnhancementIniFile = new CIniFile(g_Settings->LoadStringVal(SupportFile_Enhancements).c_str());
    m_EnhancementIniFile->SetAutoFlush(false);
    g_Settings->RegisterChangeCB(Game_IniKey, NULL, GameChanged);
    m_SectionIdent = new std::string(g_Settings->LoadStringVal(Game_IniKey));
    GameChanged(NULL);
    WriteTrace(TraceAppInit, TraceDebug, "Done");
}

void CSettingTypeEnhancement::CleanUp(void)
{
    if (m_EnhancementIniFile)
    {
        m_EnhancementIniFile->SetAutoFlush(true);
        delete m_EnhancementIniFile;
        m_EnhancementIniFile = NULL;
    }
    if (m_SectionIdent)
    {
        delete m_SectionIdent;
        m_SectionIdent = NULL;
    }
}

void CSettingTypeEnhancement::FlushChanges(void)
{
    if (m_EnhancementIniFile)
    {
        m_EnhancementIniFile->FlushChanges();
    }
}

void CSettingTypeEnhancement::GameChanged(void * /*Data */)
{
    *m_SectionIdent = g_Settings->LoadStringVal(Game_IniKey);
}

bool CSettingTypeEnhancement::IsSettingSet(void) const
{
    g_Notify->BreakPoint(__FILE__, __LINE__);
    return false;
}

bool CSettingTypeEnhancement::Load(int /*Index*/, bool & /*Value*/) const
{
    g_Notify->BreakPoint(__FILE__, __LINE__);
    return false;
}

bool CSettingTypeEnhancement::Load(int /*Index*/, uint32_t & /*Value*/) const
{
    g_Notify->BreakPoint(__FILE__, __LINE__);
    return false;
}

bool CSettingTypeEnhancement::Load(int Index, stdstr & Value) const
{
    if (m_EnhancementIniFile == NULL)
    {
        return false;
    }
    stdstr_f Key("Enhancement%d%s", Index, m_PostFix);
    return m_EnhancementIniFile->GetString(m_SectionIdent->c_str(), Key.c_str(), "", Value);
}

//return the default values
void CSettingTypeEnhancement::LoadDefault(int /*Index*/, bool & /*Value*/) const
{
    g_Notify->BreakPoint(__FILE__, __LINE__);
}

void CSettingTypeEnhancement::LoadDefault(int /*Index*/, uint32_t & /*Value*/) const
{
    g_Notify->BreakPoint(__FILE__, __LINE__);
}

void CSettingTypeEnhancement::LoadDefault(int /*Index*/, stdstr & /*Value*/) const
{
    g_Notify->BreakPoint(__FILE__, __LINE__);
}

//Update the settings
void CSettingTypeEnhancement::Save(int /*Index*/, bool /*Value*/)
{
    g_Notify->BreakPoint(__FILE__, __LINE__);
}

void CSettingTypeEnhancement::Save(int /*Index*/, uint32_t /*Value*/)
{
    g_Notify->BreakPoint(__FILE__, __LINE__);
}

void CSettingTypeEnhancement::Save(int Index, const stdstr & Value)
{
    Save(Index, Value.c_str());
}

void CSettingTypeEnhancement::Save(int Index, const char * Value)
{
    if (m_EnhancementIniFile == NULL) { return; }

    stdstr_f Key("Enhancement%d%s", Index, m_PostFix);
    if (strlen(Value) == 0)
    {
        Delete(Index);
        return;
    }
    m_EnhancementIniFile->SaveString(m_SectionIdent->c_str(), Key.c_str(), Value);
}

void CSettingTypeEnhancement::Delete(int Index)
{
    stdstr_f Key("Enhancement%d%s", Index, m_PostFix);
    m_EnhancementIniFile->SaveString(m_SectionIdent->c_str(), Key.c_str(), NULL);
}