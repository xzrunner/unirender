#pragma once

#if defined __APPLE__
    #include "TargetConditionals.h"
    #if TARGET_OS_IPHONE
        #define OPENGLES 2
        #include <OpenGLES/ES2/gl.h>
        #import <OpenGLES/ES2/glext.h>
    #elif TARGET_OS_MAC
        #define OPENGLES 3
        #include <OpenGL/gl3.h>
        #include <OpenGL/glext.h>
    #else
        #   error "Unknown Apple platform"
    #endif
#elif defined(linux) || defined(__linux) || defined(__linux__)
    #define OPENGLES 2
    #include <GLES2/gl2.h>
    #include <GLES2/gl2ext.h>
#else
    #define OPENGLES 0

    //#include <GL/glew.h>
    #include <GL/gl3w.h>
#endif
