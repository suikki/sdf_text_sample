/*
Copyright (c) 2018 Olli Kallioinen

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

//
// A wrapper for including OpenGL headers. Different OpenGL headers need
// to be used depending on the platform and OpenGL version.
//
// Defines:
//   OKGL_OPENGL_ES - 1 if OpenGL ES is used
//   OKGL_OPENGL_ES_MAJOR_VERSION - 2 or 3 when using OpenGL ES
//
//
//
// # Revision history
//      0.1   (2018-01-16) Not officially released.
//
//
// # Todo list
//   - Keycodes
//

#ifndef OKWRAPPER_OKGL_H_
#define OKWRAPPER_OKGL_H_

#include "okplatform.h"



// Always assuming OpenGL ES on mobile/web platforms.
#if OKPLATFORM_IOS || OKPLATFORM_ANDROID || OKPLATFORM_NACL || OKPLATFORM_EMSCRIPTEN || defined(OKPLATFORM_USE_OPENGL_ES)
#define OKGL_OPENGL_ES 1
#else
#define OKGL_OPENGL_ES 0
#endif


#if OKGL_OPENGL_ES
// Default to OpenGL ES 2.0 if nothing else was defined.
#ifndef OKGL_OPENGL_ES_MAJOR_VERSION
#define OKGL_OPENGL_ES_MAJOR_VERSION 2
#define OKGL_OPENGL_ES_MINOR_VERSION 0
#endif

// Currently supports only ES 2.x or ES 3.x.
#if (OKGL_OPENGL_ES_MAJOR_VERSION != 2) && (OKGL_OPENGL_ES_MAJOR_VERSION != 3)
#error "OKGL_OPENGL_ES_MAJOR_VERSION should be either 2 or 3."
#endif

#if OKPLATFORM_IOS
#if (OKGL_OPENGL_ES_MAJOR_VERSION == 2)
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#elif (OKGL_OPENGL_ES_MAJOR_VERSION == 3)
#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>
#endif

#else
#if (OKGL_OPENGL_ES_MAJOR_VERSION == 2)
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#elif (OKGL_OPENGL_ES_MAJOR_VERSION == 3)
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#endif
#endif

#endif // if OKGL_OPENGL_ES


#if !OKGL_OPENGL_ES
// When using full OpenGL -> use glew (but not using glu).
#define GLEW_NO_GLU
#include <GL/glew.h>
#endif


#endif // ifndef OKWRAPPER_OKGL_H_
