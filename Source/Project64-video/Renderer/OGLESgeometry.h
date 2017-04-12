/***************************************************************************
*                                                                          *
* Project64-video - A Nintendo 64 gfx plugin.                              *
* http://www.pj64-emu.com/                                                 *
* Copyright (C) 2017 Project64. All rights reserved.                       *
* Copyright (C) 2003-2009  Sergey 'Gonetz' Lipski                          *
* Copyright (C) 2002 Dave2001                                              *
*                                                                          *
* License:                                                                 *
* GNU/GPLv2 http://www.gnu.org/licenses/gpl-2.0.html                       *
* version 2 of the License, or (at your option) any later version.         *
*                                                                          *
****************************************************************************/
#pragma once
#include "types.h"

void gfxCullMode(gfxCullMode_t mode);
void gfxDepthBufferMode(gfxDepthBufferMode_t mode);
void gfxDepthBufferFunction(gfxCmpFnc_t function);
void gfxDepthMask(bool mask);
void gfxDepthBiasLevel(int32_t level);
void gfxDrawLine(const void *a, const void *b);
void gfxDrawTriangle(const void *a, const void *b, const void *c);
void gfxDrawVertexArray(uint32_t mode, uint32_t Count, void *pointers2);
void gfxDrawVertexArrayContiguous(uint32_t mode, uint32_t Count, void *pointers, uint32_t stride);
