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

void gfxTexCombine(gfxChipID_t tmu, gfxCombineFunction_t rgb_function, gfxCombineFunction_t rgb_factor, gfxCombineFunction_t alpha_function, gfxCombineFactor_t alpha_factor, bool rgb_invert, bool alpha_invert);
void gfxAlphaBlendFunction(gfxAlphaBlendFnc_t rgb_sf, gfxAlphaBlendFnc_t rgb_df, gfxAlphaBlendFnc_t alpha_sf, gfxAlphaBlendFnc_t alpha_df);
void gfxAlphaTestReferenceValue(gfxAlpha_t value);
void gfxAlphaTestFunction(gfxCmpFnc_t function);
void gfxFogMode(GfxFogMode_t mode);
void gfxFogGenerateLinear(float nearZ, float farZ);
void gfxFogColorValue(gfxColor_t fogcolor);
void gfxChromakeyMode(gfxChromakeyMode_t mode);
void gfxChromakeyValue(gfxColor_t value);
void gfxStippleMode(gfxStippleMode_t mode);
void gfxColorCombine(gfxCombineFunction_t function, gfxCombineFactor_t factor, gfxCombineLocal_t local, gfxCombineOther_t other, bool invert);
void gfxConstantColorValue(gfxColor_t value);
