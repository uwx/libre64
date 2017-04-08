#pragma once
#include "types.h"

void gfxColorMask(bool rgb, bool a);
void gfxClipWindow(uint32_t minx, uint32_t miny, uint32_t maxx, uint32_t maxy);
gfxContext_t gfxSstWinOpen(gfxColorFormat_t color_format, gfxOriginLocation_t origin_location, int nColBuffers, int nAuxBuffers);
bool gfxSstWinClose(gfxContext_t context);
void gfxTextureBufferExt(gfxChipID_t tmu, uint32_t startAddress, gfxLOD_t lodmin, gfxLOD_t lodmax, gfxAspectRatio_t aspect, gfxTextureFormat_t fmt, uint32_t evenOdd);
void gfxRenderBuffer(gfxBuffer_t buffer);
