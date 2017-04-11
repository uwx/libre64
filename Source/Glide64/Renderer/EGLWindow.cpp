#include "EGLWindow.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <angle/src/libEGL/libEGL.h>
#include <vector>

EGLPlatformParameters::EGLPlatformParameters()
    : renderer(EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE),
      majorVersion(EGL_DONT_CARE),
      minorVersion(EGL_DONT_CARE),
      deviceType(EGL_DONT_CARE),
      presentPath(EGL_DONT_CARE)
{
}

EGLPlatformParameters::EGLPlatformParameters(EGLint renderer)
    : renderer(renderer),
      majorVersion(EGL_DONT_CARE),
      minorVersion(EGL_DONT_CARE),
      deviceType(EGL_DONT_CARE),
      presentPath(EGL_DONT_CARE)
{
    if (renderer == EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE ||
        renderer == EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE)
    {
        deviceType = EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE;
    }
}

EGLPlatformParameters::EGLPlatformParameters(EGLint renderer,
                                             EGLint majorVersion,
                                             EGLint minorVersion,
                                             EGLint useWarp)
    : renderer(renderer),
      majorVersion(majorVersion),
      minorVersion(minorVersion),
      deviceType(useWarp),
      presentPath(EGL_DONT_CARE)
{
}

EGLPlatformParameters::EGLPlatformParameters(EGLint renderer,
                                             EGLint majorVersion,
                                             EGLint minorVersion,
                                             EGLint useWarp,
                                             EGLint presentPath)
    : renderer(renderer),
      majorVersion(majorVersion),
      minorVersion(minorVersion),
      deviceType(useWarp),
      presentPath(presentPath)
{
}

EGLWindow::EGLWindow(EGLint glesMajorVersion,
                     EGLint glesMinorVersion,
                     const EGLPlatformParameters &platform)
    : mConfig(NULL),
      mDisplay(EGL_NO_DISPLAY),
      mSurface(EGL_NO_SURFACE),
      mContext(EGL_NO_CONTEXT),
      mClientMajorVersion(glesMajorVersion),
      mClientMinorVersion(glesMinorVersion),
      mPlatform(platform),
      mRedBits(-1),
      mGreenBits(-1),
      mBlueBits(-1),
      mAlphaBits(-1),
      mDepthBits(-1),
      mStencilBits(-1),
      mMultisample(false),
      mDebug(false),
      mNoError(false),
      mSwapInterval(-1)
{
}

EGLWindow::~EGLWindow()
{
    destroyGL();
}

void EGLWindow::swap()
{
    eglSwapBuffers(mDisplay, mSurface);
}

bool EGLWindow::initializeGL(void * hwnd)
{
    std::vector<EGLint> displayAttributes;
    displayAttributes.push_back(EGL_PLATFORM_ANGLE_TYPE_ANGLE);
    displayAttributes.push_back(mPlatform.renderer);
    displayAttributes.push_back(EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE);
    displayAttributes.push_back(mPlatform.majorVersion);
    displayAttributes.push_back(EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE);
    displayAttributes.push_back(mPlatform.minorVersion);

    if (mPlatform.deviceType != EGL_DONT_CARE)
    {
        displayAttributes.push_back(EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE);
        displayAttributes.push_back(mPlatform.deviceType);
    }

    if (mPlatform.presentPath != EGL_DONT_CARE)
    {
        const char *extensionString =
            static_cast<const char *>(eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS));
        if (strstr(extensionString, "EGL_ANGLE_experimental_present_path") == nullptr)
        {
            destroyGL();
            return false;
        }

        displayAttributes.push_back(EGL_EXPERIMENTAL_PRESENT_PATH_ANGLE);
        displayAttributes.push_back(mPlatform.presentPath);
    }
    displayAttributes.push_back(EGL_NONE);

    mDisplay = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, reinterpret_cast<void *>(GetDC((HWND)hwnd)), &displayAttributes[0]);
    if (mDisplay == EGL_NO_DISPLAY)
    {
        destroyGL();
        return false;
    }

    EGLint majorVersion, minorVersion;
    if (eglInitialize(mDisplay, &majorVersion, &minorVersion) == EGL_FALSE)
    {
        destroyGL();
        return false;
    }

    const char *displayExtensions = eglQueryString(mDisplay, EGL_EXTENSIONS);

    // EGL_KHR_create_context is required to request a non-ES2 context.
    bool hasKHRCreateContext = strstr(displayExtensions, "EGL_KHR_create_context") != nullptr;
    if (majorVersion != 2 && minorVersion != 0 && !hasKHRCreateContext)
    {
        destroyGL();
        return false;
    }

    eglBindAPI(EGL_OPENGL_ES_API);
    if (eglGetError() != EGL_SUCCESS)
    {
        destroyGL();
        return false;
    }

    const EGLint configAttributes[] =
    {
        EGL_RED_SIZE,       (mRedBits >= 0)     ? mRedBits     : EGL_DONT_CARE,
        EGL_GREEN_SIZE,     (mGreenBits >= 0)   ? mGreenBits   : EGL_DONT_CARE,
        EGL_BLUE_SIZE,      (mBlueBits >= 0)    ? mBlueBits    : EGL_DONT_CARE,
        EGL_ALPHA_SIZE,     (mAlphaBits >= 0)   ? mAlphaBits   : EGL_DONT_CARE,
        EGL_DEPTH_SIZE,     (mDepthBits >= 0)   ? mDepthBits   : EGL_DONT_CARE,
        EGL_STENCIL_SIZE,   (mStencilBits >= 0) ? mStencilBits : EGL_DONT_CARE,
        EGL_SAMPLE_BUFFERS, mMultisample ? 1 : 0,
        EGL_NONE
    };

    EGLint configCount;
    if (!eglChooseConfig(mDisplay, configAttributes, &mConfig, 1, &configCount) || (configCount != 1))
    {
        destroyGL();
        return false;
    }

    eglGetConfigAttrib(mDisplay, mConfig, EGL_RED_SIZE, &mRedBits);
    eglGetConfigAttrib(mDisplay, mConfig, EGL_GREEN_SIZE, &mGreenBits);
    eglGetConfigAttrib(mDisplay, mConfig, EGL_BLUE_SIZE, &mBlueBits);
    eglGetConfigAttrib(mDisplay, mConfig, EGL_ALPHA_SIZE, &mAlphaBits);
    eglGetConfigAttrib(mDisplay, mConfig, EGL_DEPTH_SIZE, &mDepthBits);
    eglGetConfigAttrib(mDisplay, mConfig, EGL_STENCIL_SIZE, &mStencilBits);

    std::vector<EGLint> surfaceAttributes;
    if (strstr(displayExtensions, "EGL_NV_post_sub_buffer") != nullptr)
    {
        surfaceAttributes.push_back(EGL_POST_SUB_BUFFER_SUPPORTED_NV);
        surfaceAttributes.push_back(EGL_TRUE);
    }

    surfaceAttributes.push_back(EGL_NONE);

    mSurface = eglCreateWindowSurface(mDisplay, mConfig, (HWND)hwnd, &surfaceAttributes[0]);
    if (eglGetError() != EGL_SUCCESS)
    {
        destroyGL();
        return false;
    }

    std::vector<EGLint> contextAttributes;
    if (hasKHRCreateContext)
    {
        contextAttributes.push_back(EGL_CONTEXT_MAJOR_VERSION_KHR);
        contextAttributes.push_back(mClientMajorVersion);

        contextAttributes.push_back(EGL_CONTEXT_MINOR_VERSION_KHR);
        contextAttributes.push_back(mClientMinorVersion);

        contextAttributes.push_back(EGL_CONTEXT_OPENGL_DEBUG);
        contextAttributes.push_back(mDebug ? EGL_TRUE : EGL_FALSE);

        // TODO(jmadill): Check for the extension string.
        // bool hasKHRCreateContextNoError = strstr(displayExtensions,
        // "EGL_KHR_create_context_no_error") != nullptr;

        contextAttributes.push_back(EGL_CONTEXT_OPENGL_NO_ERROR_KHR);
        contextAttributes.push_back(mNoError ? EGL_TRUE : EGL_FALSE);
    }
    contextAttributes.push_back(EGL_NONE);

    mContext = eglCreateContext(mDisplay, mConfig, nullptr, &contextAttributes[0]);
    if (eglGetError() != EGL_SUCCESS)
    {
        destroyGL();
        return false;
    }

    eglMakeCurrent(mDisplay, mSurface, mSurface, mContext);
    if (eglGetError() != EGL_SUCCESS)
    {
        destroyGL();
        return false;
    }

    if (mSwapInterval != -1)
    {
        eglSwapInterval(mDisplay, mSwapInterval);
    }

    return true;
}

void EGLWindow::destroyGL()
{
    if (mSurface != EGL_NO_SURFACE)
    {
        eglDestroySurface(mDisplay, mSurface);
        mSurface = EGL_NO_SURFACE;
    }

    if (mContext != EGL_NO_CONTEXT)
    {
        eglDestroyContext(mDisplay, mContext);
        mContext = EGL_NO_CONTEXT;
    }

    if (mDisplay != EGL_NO_DISPLAY)
    {
        eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglTerminate(mDisplay);
        mDisplay = EGL_NO_DISPLAY;
    }
}
