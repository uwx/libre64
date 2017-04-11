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

typedef struct DRAWIMAGE_t {
    float frameX;
    float frameY;
    uint16_t frameW;
    uint16_t frameH;
    uint16_t imageX;
    uint16_t imageY;
    uint16_t imageW;
    uint16_t imageH;
    uint32_t imagePtr;
    uint8_t imageFmt;
    uint8_t imageSiz;
    uint16_t imagePal;
    uint8_t flipX;
    uint8_t flipY;
    float scaleX;
    float scaleY;
} DRAWIMAGE;

void uc6_bg_1cyc();
void uc6_bg_copy();
void uc6_loaducode();
void uc6_ldtx_rect_r();
void uc6_obj_ldtx_rect();
void uc6_obj_ldtx_sprite();
void uc6_obj_loadtxtr();
void uc6_obj_movemem();
void uc6_obj_rectangle();
void uc6_obj_rectangle_r();
void uc6_obj_rendermode();
void uc6_obj_sprite();
void uc6_select_dl();
void uc6_sprite2d();

void DrawHiresImage(DRAWIMAGE & d, int screensize = FALSE);
float set_sprite_combine_mode();
