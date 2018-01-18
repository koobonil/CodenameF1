#pragma once

#include "zay_types.h"

#ifdef BOSS_PLATFORM_QT5
    #define USE_QT_OPENGL 1
#else
    #define USE_QT_OPENGL 0
#endif

#include <platform/boss_platform.hpp>
#if USE_QT_OPENGL
    #include <QGLFunctions>
    extern QGLFunctions* g_func;
    #define BOSS_GL(NAME, ...) g_func->gl##NAME(__VA_ARGS__)
#else
    # if BOSS_WINDOWS
    #  include "glew-1.12.0/include/GL/glew.h"
    # elif BOSS_MAC_OSX
    #  include <OpenGL/gl3.h>
    #  include <OpenGL/gl3ext.h>
    # elif BOSS_IPHONE
    #  include <OpenGLES/ES3/gl.h>
    #  include <OpenGLES/ES3/glext.h>
    #  ifdef __OBJC__
    #   import <OpenGLES/EAGLDrawable.h>
    #  endif
    # elif BOSS_ANDROID
    #  include <EGL/egl.h>
    #  include <android/native_window_jni.h>
    #  include "platform/android/OpenGLES_Functions-android.h"
    # endif

    extern dependency* g_func;
    #define BOSS_GL(NAME, ...) gl##NAME(__VA_ARGS__)
#endif

namespace ZAY
{
    struct PixelFormatInfo
    {
        PixelFormatInfo(GLenum anInternalFormat, GLenum aFormat, GLenum aType, int32_t aBpp, bool aCompressed, bool anAlpha)
        : internalFormat(anInternalFormat)
        , format(aFormat)
        , type(aType)
        , bpp(aBpp)
        , compressed(aCompressed)
        , alpha(anAlpha)
        {}

        GLenum internalFormat;
        GLenum format;
        GLenum type;
        int32_t bpp;
        bool compressed;
        bool alpha;
    };

    typedef std::map<PixelFormat, const PixelFormatInfo> PixelFormatInfoMap;

    struct TexParams
    {
        GLuint    minFilter;
        GLuint    magFilter;
        GLuint    wrapS;
        GLuint    wrapT;
    };
}
