/****************************************************************************
*                                                                           *
* Project64 - A Nintendo 64 emulator.                                       *
* http://www.pj64-emu.com/                                                  *
* Copyright (C) 2012 Project64. All rights reserved.                        *
*                                                                           *
* License:                                                                  *
* GNU/GPLv2 http://www.gnu.org/licenses/gpl-2.0.html                        *
*                                                                           *
****************************************************************************/
#pragma once

void uc1_branch_z();
void uc1_rdphalf_1();
void uc1_line3d();
void uc1_tri1();
void uc1_tri2();
void uc1_vertex();

extern uint32_t branch_dl;
