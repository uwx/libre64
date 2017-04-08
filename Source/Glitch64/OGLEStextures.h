#pragma once
#include "types.h"

uint32_t gfxTexMinAddress(gfxChipID_t tmu);
uint32_t gfxTexMaxAddress(gfxChipID_t tmu);
uint32_t gfxTexCalcMemRequired(gfxLOD_t lodmin, gfxLOD_t lodmax, gfxAspectRatio_t aspect, gfxTextureFormat_t fmt);
uint32_t gfxTexTextureMemRequired(uint32_t evenOdd, gfxTexInfo *info);
void gfxTexDownloadMipMap(gfxChipID_t tmu, uint32_t startAddress, uint32_t evenOdd, gfxTexInfo *info);
void gfxTexSource(gfxChipID_t tmu, uint32_t startAddress, uint32_t evenOdd, gfxTexInfo *info);
void gfxTexDetailControl(gfxChipID_t tmu, int lod_bias, uint8_t detail_scale, float detail_max);
void gfxTexFilterMode(gfxChipID_t tmu, gfxTextureFilterMode_t minfilter_mode, gfxTextureFilterMode_t magfilter_mode);
void gfxTexClampMode(gfxChipID_t tmu, gfxTextureClampMode_t s_clampmode, gfxTextureClampMode_t t_clampmode);
