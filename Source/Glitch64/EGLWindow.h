#pragma once

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