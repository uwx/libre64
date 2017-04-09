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
