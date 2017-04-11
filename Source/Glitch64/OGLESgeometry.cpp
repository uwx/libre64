/*
* Glide64 - Glide video plugin for Nintendo 64 emulators.
* Copyright (c) 2002  Dave2001
* Copyright (c) 2003-2009  Sergey 'Gonetz' Lipski
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#endif // _WIN32
#include "glide.h"
#include "glitchmain.h"
#include "../Glide64/winlnxdefs.h"
#include "../Glide64/rdp.h"
#include <Glide64/trace.h>

#define Z_MAX (65536.0f)
#define VERTEX_SIZE sizeof(VERTEX) //Size of vertex struct

int w_buffer_mode;
int inverted_culling;
gfxCullMode_t culling_mode;

#define VERTEX_BUFFER_SIZE 1500 //Max amount of vertices to buffer, this seems large enough.
static VERTEX vertex_buffer[VERTEX_BUFFER_SIZE];
static int vertex_buffer_count = 0;
static GLenum vertex_draw_mode;
static bool vertex_buffer_enabled = false;

void vbo_draw()
{
    if (vertex_buffer_count)
    {
        WriteTrace(TraceGlide64, TraceDebug, "vertex_draw_mode: %d vertex_buffer_count: %d", vertex_draw_mode, vertex_buffer_count);
        glDrawArrays(vertex_draw_mode, 0, vertex_buffer_count);
        vertex_buffer_count = 0;
        WriteTrace(TraceGlide64, TraceDebug, "done (glGetError() = %X)", glGetError());
    }
}

//Buffer vertices instead of glDrawArrays(...)
void vbo_buffer(GLenum mode, GLint first, GLsizei count, void* pointers)
{
    if ((count != 3 && mode != GL_TRIANGLES) || vertex_buffer_count + count > VERTEX_BUFFER_SIZE)
    {
        vbo_draw();
    }

    memcpy(&vertex_buffer[vertex_buffer_count], pointers, count * VERTEX_SIZE);
    vertex_buffer_count += count;

    if (count == 3 || mode == GL_TRIANGLES)
    {
        vertex_draw_mode = GL_TRIANGLES;
    }
    else
    {
        vertex_draw_mode = mode;
        vbo_draw(); //Triangle fans and strips can't be joined as easily, just draw them straight away.
    }
}

void vbo_enable()
{
    if (vertex_buffer_enabled)
        return;

    vertex_buffer_enabled = true;
    glEnableVertexAttribArray(POSITION_ATTR);
    glVertexAttribPointer(POSITION_ATTR, 4, GL_FLOAT, GL_FALSE, VERTEX_SIZE, &vertex_buffer[0].x); //Position

    glEnableVertexAttribArray(COLOUR_ATTR);
    glVertexAttribPointer(COLOUR_ATTR, 4, GL_UNSIGNED_BYTE, GL_TRUE, VERTEX_SIZE, &vertex_buffer[0].b); //Colour

    glEnableVertexAttribArray(TEXCOORD_0_ATTR);
    glVertexAttribPointer(TEXCOORD_0_ATTR, 2, GL_FLOAT, false, VERTEX_SIZE, &vertex_buffer[0].coord[2]); //Tex0

    glEnableVertexAttribArray(TEXCOORD_1_ATTR);
    glVertexAttribPointer(TEXCOORD_1_ATTR, 2, GL_FLOAT, false, VERTEX_SIZE, &vertex_buffer[0].coord[0]); //Tex1

    glEnableVertexAttribArray(FOG_ATTR);
    glVertexAttribPointer(FOG_ATTR, 1, GL_FLOAT, false, VERTEX_SIZE, &vertex_buffer[0].f); //Fog
}

void vbo_disable()
{
    vbo_draw();
    vertex_buffer_enabled = false;
}

inline float ZCALC(const float & z, const float & q)
{
    float res = ((z) / Z_MAX) / (q);
    return res;
}

static inline float ytex(int tmu, float y)
{
    return invtex[tmu] ? invtex[tmu] - y : y;
}

void init_geometry()
{
    w_buffer_mode = 0;
    inverted_culling = 0;

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
}

void gfxCullMode(gfxCullMode_t mode)
{
    WriteTrace(TraceGlitch, TraceDebug, "mode: %d", mode);
    static int oldmode = -1, oldinv = -1;
    culling_mode = mode;
    if (inverted_culling == oldinv && oldmode == mode)
    {
        return;
    }
    oldmode = mode;
    oldinv = inverted_culling;
    switch (mode)
    {
    case GFX_CULL_DISABLE:
        glDisable(GL_CULL_FACE);
        break;
    case GFX_CULL_NEGATIVE:
        if (!inverted_culling)
            glCullFace(GL_FRONT);
        else
            glCullFace(GL_BACK);
        glEnable(GL_CULL_FACE);
        break;
    case GFX_CULL_POSITIVE:
        if (!inverted_culling)
            glCullFace(GL_BACK);
        else
            glCullFace(GL_FRONT);
        glEnable(GL_CULL_FACE);
        break;
    default:
        WriteTrace(TraceGlitch, TraceWarning, "unknown cull mode : %x", mode);
    }
}

// Depth buffer

void gfxDepthBufferMode(gfxDepthBufferMode_t mode)
{
    WriteTrace(TraceGlitch, TraceDebug, "mode: %d", mode);
    switch (mode)
    {
    case GFX_DEPTHBUFFER_DISABLE:
        glDisable(GL_DEPTH_TEST);
        w_buffer_mode = 0;
        return;
    case GFX_DEPTHBUFFER_WBUFFER:
    case GFX_DEPTHBUFFER_WBUFFER_COMPARE_TO_BIAS:
        glEnable(GL_DEPTH_TEST);
        w_buffer_mode = 1;
        break;
    case GFX_DEPTHBUFFER_ZBUFFER:
    case GFX_DEPTHBUFFER_ZBUFFER_COMPARE_TO_BIAS:
        glEnable(GL_DEPTH_TEST);
        w_buffer_mode = 0;
        break;
    default:
        WriteTrace(TraceGlitch, TraceWarning, "unknown depth buffer mode : %x", mode);
    }
}

void gfxDepthBufferFunction(gfxCmpFnc_t function)
{
    WriteTrace(TraceGlitch, TraceDebug, "function: %d", function);
    switch (function)
    {
    case GFX_CMP_GEQUAL:
        if (w_buffer_mode)
            glDepthFunc(GL_LEQUAL);
        else
            glDepthFunc(GL_GEQUAL);
        break;
    case GFX_CMP_LEQUAL:
        if (w_buffer_mode)
            glDepthFunc(GL_GEQUAL);
        else
            glDepthFunc(GL_LEQUAL);
        break;
    case GFX_CMP_LESS:
        if (w_buffer_mode)
            glDepthFunc(GL_GREATER);
        else
            glDepthFunc(GL_LESS);
        break;
    case GFX_CMP_ALWAYS:
        glDepthFunc(GL_ALWAYS);
        break;
    case GFX_CMP_EQUAL:
        glDepthFunc(GL_EQUAL);
        break;
    case GFX_CMP_GREATER:
        if (w_buffer_mode)
            glDepthFunc(GL_LESS);
        else
            glDepthFunc(GL_GREATER);
        break;
    case GFX_CMP_NEVER:
        glDepthFunc(GL_NEVER);
        break;
    case GFX_CMP_NOTEQUAL:
        glDepthFunc(GL_NOTEQUAL);
        break;

    default:
        WriteTrace(TraceGlitch, TraceWarning, "unknown depth buffer function : %x", function);
    }
}

void gfxDepthMask(bool mask)
{
    WriteTrace(TraceGlitch, TraceDebug, "mask: %d", mask);
    glDepthMask(mask);
}

void gfxDepthBiasLevel(int32_t level)
{
    WriteTrace(TraceGlitch, TraceDebug, "level: %d", level);
    if (level)
    {
        if (w_buffer_mode)
        {
            glPolygonOffset(1.0f, -(float)level*zscale / 255.0f);
        }
        else
        {
            glPolygonOffset(0, 0);
        }
        glEnable(GL_POLYGON_OFFSET_FILL);
    }
    else
    {
        glPolygonOffset(0, 0);
        glDisable(GL_POLYGON_OFFSET_FILL);
    }
}

void gfxDrawTriangle(const void *a, const void *b, const void *c)
{
    WriteTrace(TraceGlitch, TraceDebug, "start");
    vbo_enable();

    reloadTexture();

    if (need_to_compile) compile_shader();

    if (vertex_buffer_count + 3 > VERTEX_BUFFER_SIZE)
    {
        vbo_draw();
    }
    vertex_draw_mode = GL_TRIANGLES;
    memcpy(&vertex_buffer[vertex_buffer_count], a, VERTEX_SIZE);
    memcpy(&vertex_buffer[vertex_buffer_count + 1], b, VERTEX_SIZE);
    memcpy(&vertex_buffer[vertex_buffer_count + 2], c, VERTEX_SIZE);
    vertex_buffer_count += 3;

    WriteTrace(TraceGlitch, TraceDebug, "Done");
}

void gfxDrawLine(const void *a, const void *b)
{
}

void gfxDrawVertexArray(uint32_t mode, uint32_t Count, void *pointers2)
{
    void **pointers = (void**)pointers2;
    WriteTrace(TraceGlitch, TraceDebug, "gfxDrawVertexArray(%d,%d)\r\n", mode, Count);

    reloadTexture();

    if (need_to_compile) compile_shader();

    if (mode != GR_TRIANGLE_FAN)
    {
        WriteTrace(TraceGlitch, TraceWarning, "gfxDrawVertexArray : unknown mode : %x", mode);
    }

    vbo_enable();
    vbo_buffer(GL_TRIANGLE_FAN, 0, Count, pointers[0]);
}

void gfxDrawVertexArrayContiguous(uint32_t mode, uint32_t Count, void *pointers, uint32_t stride)
{
    WriteTrace(TraceGlitch, TraceDebug, "gfxDrawVertexArrayContiguous(%d,%d,%d)\r\n", mode, Count, stride);

    if (stride != 156)
    {
        //LOGINFO("Incompatible stride\n");
    }

    reloadTexture();

    if (need_to_compile) compile_shader();

    vbo_enable();

    switch (mode)
    {
    case GR_TRIANGLE_STRIP:
        vbo_buffer(GL_TRIANGLE_STRIP, 0, Count, pointers);
        break;
    case GR_TRIANGLE_FAN:
        vbo_buffer(GL_TRIANGLE_FAN, 0, Count, pointers);
        break;
    default:
        WriteTrace(TraceGlitch, TraceWarning, "gfxDrawVertexArrayContiguous : unknown mode : %x", mode);
    }
}