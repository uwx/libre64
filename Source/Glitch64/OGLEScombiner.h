#pragma once
#include <Common/stdtypes.h>

typedef int32_t gfxCmpFnc_t;
typedef uint32_t gfxColor_t;
typedef int32_t gfxCombineFunction_t;
typedef int32_t gfxCombineFactor_t;
typedef int32_t gfxCombineLocal_t;
typedef int32_t gfxCombineOther_t;
typedef int32_t gfxStippleMode_t;
typedef int32_t gfxChromakeyMode_t;
typedef int32_t GfxFogMode_t;

void gfxAlphaTestFunction(gfxCmpFnc_t function);
void gfxFogMode(GfxFogMode_t mode);
void gfxFogGenerateLinear(float nearZ, float farZ);
void gfxFogColorValue(gfxColor_t fogcolor);
void gfxChromakeyMode(gfxChromakeyMode_t mode);
void gfxChromakeyValue(gfxColor_t value);
void gfxStippleMode(gfxStippleMode_t mode);
void gfxColorCombine(gfxCombineFunction_t function, gfxCombineFactor_t factor, gfxCombineLocal_t local, gfxCombineOther_t other, bool invert);
void gfxConstantColorValue(gfxColor_t value);
