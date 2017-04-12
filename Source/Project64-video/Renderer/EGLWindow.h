/****************************************************************************
*                                                                           *
* Project64-video - A Nintendo 64 gfx plugin.                               *
* http://www.pj64-emu.com/                                                  *
* Copyright (C) 2017 Project64. All rights reserved.                        *
*                                                                           *
* License:                                                                  *
* GNU/GPLv2 http://www.gnu.org/licenses/gpl-2.0.html                        *
*                                                                           *
****************************************************************************/
#pragma once
#ifdef _WIN32

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

struct EGLPlatformParameters
{
    EGLint renderer;
    EGLint majorVersion;
    EGLint minorVersion;
    EGLint deviceType;
    EGLint presentPath;

    EGLPlatformParameters();
    explicit EGLPlatformParameters(EGLint renderer);
    EGLPlatformParameters(EGLint renderer, EGLint majorVersion, EGLint minorVersion, EGLint deviceType);
    EGLPlatformParameters(EGLint renderer, EGLint majorVersion, EGLint minorVersion, EGLint deviceType, EGLint presentPath);
};

class EGLWindow
{
public:
    EGLWindow(EGLint glesMajorVersion,
        EGLint glesMinorVersion,
        const EGLPlatformParameters &platform);

    ~EGLWindow();

    void setConfigRedBits(int bits) { mRedBits = bits; }
    void setConfigGreenBits(int bits) { mGreenBits = bits; }
    void setConfigBlueBits(int bits) { mBlueBits = bits; }
    void setConfigAlphaBits(int bits) { mAlphaBits = bits; }
    void setConfigDepthBits(int bits) { mDepthBits = bits; }
    void setConfigStencilBits(int bits) { mStencilBits = bits; }
    void setSwapInterval(EGLint swapInterval) { mSwapInterval = swapInterval; }
    void swap();

    bool initializeGL(void * hwnd);
    void destroyGL();

private:
    EGLConfig mConfig;
    EGLDisplay mDisplay;
    EGLSurface mSurface;
    EGLContext mContext;

    EGLint mClientMajorVersion;
    EGLint mClientMinorVersion;
    EGLPlatformParameters mPlatform;
    int mRedBits;
    int mGreenBits;
    int mBlueBits;
    int mAlphaBits;
    int mDepthBits;
    int mStencilBits;
    bool mMultisample;
    bool mDebug;
    bool mNoError;
    EGLint mSwapInterval;
};

#endif
