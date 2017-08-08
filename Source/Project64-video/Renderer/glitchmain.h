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

#include <Project64-video/trace.h>

#define zscale 1.0f

extern int packed_pixels_support;
extern float largest_supported_anisotropy;

extern int default_texture; // the infamous "32*1024*1024" is now configurable
extern int depth_texture;
extern float invtex[2];
extern int buffer_cleared; // mark that the buffer has been cleared, used to check if we need to reload the texture buffer content

#ifndef _WIN32
#include <stdio.h>
#endif

#include "OGLESwrappers.h"
#include "types.h"

void init_geometry();
bool init_textures();
void init_combiner();
void free_textures();
void remove_tex(unsigned int idmin, unsigned int idmax);
void add_tex(unsigned int id);

void check_link(GLuint program);
void vbo_enable();
void vbo_disable();

//Vertex Attribute Locations
enum VERTEX_ATTRIBUTE_LOCATIONS
{
    POSITION_ATTR = 0,
    COLOUR_ATTR = 1,
    TEXCOORD_0_ATTR = 2,
    TEXCOORD_1_ATTR = 3,
    FOG_ATTR = 4,
};

extern int w_buffer_mode;
extern int g_width, g_height, widtho, heighto;
extern int tex0_width, tex0_height, tex1_width, tex1_height;
extern float lambda;
extern int inverted_culling;
extern gfxCullMode_t culling_mode;
extern int render_to_texture;
extern int lfb_color_fmt;
extern int need_to_compile;
extern int blackandwhite0;
extern int blackandwhite1;

extern int blend_func_separate_support;
extern unsigned int pBufferAddress;
extern int viewport_width, viewport_height, g_viewport_offset;

void updateTexture();
void reloadTexture();
void free_combiners();
void compile_shader();
void set_lambda();
void set_copy_shader();
void disable_textureSizes();
void ExitFullScreen();
