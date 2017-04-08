#pragma once
#include <Common/stdtypes.h>

typedef int32_t gfxChipID_t;
typedef int32_t gfxAlphaBlendFnc_t;
typedef uint8_t gfxAlpha_t;
typedef int32_t gfxCmpFnc_t;
typedef uint32_t gfxColor_t;
typedef int32_t gfxCombineFunction_t;
typedef int32_t gfxCombineFactor_t;
typedef int32_t gfxCombineLocal_t;
typedef int32_t gfxCombineOther_t;
typedef int32_t gfxStippleMode_t;
typedef int32_t gfxChromakeyMode_t;
typedef int32_t GfxFogMode_t;
typedef int32_t gfxTextureClampMode_t;
typedef int32_t gfxTextureFilterMode_t;
typedef int32_t gfxLOD_t;
typedef int32_t gfxAspectRatio_t;
typedef int32_t gfxTextureFormat_t;
typedef int32_t gfxContext_t;
typedef int32_t gfxColorFormat_t;
typedef int32_t gfxOriginLocation_t;

typedef struct {
    gfxLOD_t           smallLodLog2;
    gfxLOD_t           largeLodLog2;
    gfxAspectRatio_t   aspectRatioLog2;
    gfxTextureFormat_t format;
    void               *data;
} gfxTexInfo;