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
typedef int32_t gfxAspectRatio_t;
typedef int32_t gfxTextureFormat_t;
typedef int32_t gfxContext_t;
typedef int32_t gfxColorFormat_t;
typedef int32_t gfxOriginLocation_t;
typedef int32_t gfxLock_t;
typedef int32_t gfxLfbWriteMode_t;
typedef uint32_t gfxLfbSrcFmt_t;
typedef int32_t gfxCullMode_t;
typedef int32_t gfxDepthBufferMode_t;
typedef uint32_t gfxCCUColor_t;
typedef uint32_t gfxACUColor_t;
typedef uint32_t gfxTCCUColor_t;
typedef uint32_t gfxTACUColor_t;

enum gfxCombineMode_t
{
    GFX_FUNC_MODE_ZERO = 0x00,
    GFX_FUNC_MODE_X = 0x01,
    GFX_FUNC_MODE_ONE_MINUS_X = 0x02,
    GFX_FUNC_MODE_NEGATIVE_X = 0x03,
    GFX_FUNC_MODE_X_MINUS_HALF = 0x04,
};

enum gfxLOD_t
{
    GFX_LOD_LOG2_1 = 0x0,
    GFX_LOD_LOG2_2 = 0x1,
    GFX_LOD_LOG2_4 = 0x2,
    GFX_LOD_LOG2_8 = 0x3,
    GFX_LOD_LOG2_16 = 0x4,
    GFX_LOD_LOG2_32 = 0x5,
    GFX_LOD_LOG2_64 = 0x6,
    GFX_LOD_LOG2_128 = 0x7,
    GFX_LOD_LOG2_256 = 0x8,
    GFX_LOD_LOG2_512 = 0x9,
    GFX_LOD_LOG2_1024 = 0xa,
    GFX_LOD_LOG2_2048 = 0xb,
};

enum gfxBuffer_t
{
    GFX_BUFFER_FRONTBUFFER = 0x0,
    GFX_BUFFER_BACKBUFFER = 0x1,
    GFX_BUFFER_AUXBUFFER = 0x2,
    GFX_BUFFER_DEPTHBUFFER = 0x3,
    GFX_BUFFER_ALPHABUFFER = 0x4,
    GFX_BUFFER_TRIPLEBUFFER = 0x5,
    GFX_BUFFER_TEXTUREBUFFER_EXT = 0x6,
    GFX_BUFFER_TEXTUREAUXBUFFER_EXT = 0x7,
};

typedef struct {
    gfxLOD_t           smallLodLog2;
    gfxLOD_t           largeLodLog2;
    gfxAspectRatio_t   aspectRatioLog2;
    gfxTextureFormat_t format;
    void               *data;
} gfxTexInfo;

typedef struct {
    int32_t            size;
    void               *lfbPtr;
    uint32_t           strideInBytes;
    gfxLfbWriteMode_t   writeMode;
    gfxOriginLocation_t origin;
} gfxLfbInfo_t;

enum GFX_CMBX
{
    GFX_CMBX_ZERO = 0x00,
    GFX_CMBX_TEXTURE_ALPHA = 0x01,
    GFX_CMBX_ALOCAL = 0x02,
    GFX_CMBX_AOTHER = 0x03,
    GFX_CMBX_B = 0x04,
    GFX_CMBX_CONSTANT_ALPHA = 0x05,
    GFX_CMBX_CONSTANT_COLOR = 0x06,
    GFX_CMBX_DETAIL_FACTOR = 0x07,
    GFX_CMBX_ITALPHA = 0x08,
    GFX_CMBX_ITRGB = 0x09,
    GFX_CMBX_LOCAL_TEXTURE_ALPHA = 0x0a,
    GFX_CMBX_LOCAL_TEXTURE_RGB = 0x0b,
    GFX_CMBX_LOD_FRAC = 0x0c,
    GFX_CMBX_OTHER_TEXTURE_ALPHA = 0x0d,
    GFX_CMBX_OTHER_TEXTURE_RGB = 0x0e,
    GFX_CMBX_TEXTURE_RGB = 0x0f,
    GFX_CMBX_TMU_CALPHA = 0x10,
    GFX_CMBX_TMU_CCOLOR = 0x11,
};