#pragma once
#include "types.h"

void gfxTexDetailControl(gfxChipID_t tmu, int lod_bias, uint8_t detail_scale, float detail_max);
void gfxTexFilterMode(gfxChipID_t tmu, gfxTextureFilterMode_t minfilter_mode, gfxTextureFilterMode_t magfilter_mode);
void gfxTexClampMode(gfxChipID_t tmu, gfxTextureClampMode_t s_clampmode, gfxTextureClampMode_t t_clampmode);
