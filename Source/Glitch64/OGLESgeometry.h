#pragma once
#include "types.h"

void gfxCullMode(gfxCullMode_t mode);
void gfxDepthBufferMode(gfxDepthBufferMode_t mode);
void gfxDepthBufferFunction(gfxCmpFnc_t function);
void gfxDepthMask(bool mask);
void gfxDepthBiasLevel(int32_t level);
