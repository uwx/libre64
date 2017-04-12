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

void gfxAlphaCombine(gfxCombineFunction_t function, gfxCombineFactor_t factor, gfxCombineLocal_t local, gfxCombineOther_t other, bool invert);
void gfxTexCombine(gfxChipID_t tmu, gfxCombineFunction_t rgb_function, gfxCombineFactor_t rgb_factor, gfxCombineFunction_t alpha_function, gfxCombineFactor_t alpha_factor, bool rgb_invert, bool alpha_invert);
void gfxAlphaBlendFunction(gfxAlphaBlendFnc_t rgb_sf, gfxAlphaBlendFnc_t rgb_df, gfxAlphaBlendFnc_t alpha_sf, gfxAlphaBlendFnc_t alpha_df);
void gfxAlphaTestReferenceValue(gfxAlpha_t value);
void gfxAlphaTestFunction(gfxCmpFnc_t function);
void gfxFogMode(GfxFogMode_t mode);
void gfxFogGenerateLinear(float nearZ, float farZ);
void gfxFogColorValue(gfxColor_t fogcolor);
void gfxChromakeyMode(gfxChromakeyMode_t mode);
void gfxChromakeyValue(gfxColor_t value);
void gfxStippleMode(gfxStippleMode_t mode);
void gfxColorCombine(gfxCombineFunction_t function, gfxCombineFactor_t factor, gfxCombineLocal_t local, gfxCombineOther_t other, bool invert);
void gfxConstantColorValue(gfxColor_t value);
