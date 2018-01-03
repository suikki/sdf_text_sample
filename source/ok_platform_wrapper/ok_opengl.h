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

#ifndef PLATFORM_WRAPPER_OK_OPENGL_H_
#define PLATFORM_WRAPPER_OK_OPENGL_H_

#include "ok_platform.h"

/*
 * Different OpenGL headers need to be loaded depending on platform and OpenGL version.
 *
 * Defines:
 *   OK_OPENGL_ES - if OpenGL ES is used
 *   OK_OPENGL_ES_MAJOR_VERSION - 2 or 3 when using OK_OPENGL_ES
 */

/* Assuming OpenGL ES on mobile/web platforms. */
#if defined(OK_PLATFORM_IOS) || defined(OK_PLATFORM_ANDROID) || defined(OK_PLATFORM_NACL) || defined(OK_PLATFORM_EMSCRIPTEN)
#define OK_OPENGL_ES
#endif


#ifdef OK_OPENGL_ES
/* Default to OpenGL ES 2.0 if nothing else was defined */
#ifndef OK_OPENGL_ES_MAJOR_VERSION
#define OK_OPENGL_ES_MAJOR_VERSION 2
#define OK_OPENGL_ES_MINOR_VERSION 0
#endif

/* Currently supports only ES 2.x and ES 3.x */
#if (OK_OPENGL_ES_MAJOR_VERSION != 2) && (OK_OPENGL_ES_MAJOR_VERSION != 3)
#error "OK_OPENGL_ES_MAJOR_VERSION should be either 2 or 3."
#endif

#if defined(OK_PLATFORM_IOS)

#if (OK_OPENGL_ES_MAJOR_VERSION == 2)
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#elif (OK_OPENGL_ES_MAJOR_VERSION == 3)
#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>
#endif

#else

#if (OK_OPENGL_ES_MAJOR_VERSION == 2)
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#elif (OK_OPENGL_ES_MAJOR_VERSION == 3)
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#endif

#endif

#endif /* ifdef OK_OPENGL_ES */


/* If using full OpenGL -> use glew. */
#ifndef OK_OPENGL_ES

/* Not using glu. */
#define GLEW_NO_GLU

#include <GL/glew.h>

#endif


#endif /* ifndef PLATFORM_WRAPPER_OK_OPENGL_H_ */