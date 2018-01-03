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
// Using a separate compilation unit for header only library implementations
// to not pollute the app too much.
//

#define OKLOG_IMPLEMENTATION
#define OKAPP_IMPLEMENTATION
#define OKGL_IMPLEMENTATION

#include "ok_platform_wrapper/ok_log.h"
#include "ok_platform_wrapper/ok_app.h"
#include "ok_gl_helper/ok_gl_helper.h"


//
// Fontstash.
//
#define FONTSTASH_IMPLEMENTATION

#if (OK_OPENGL_ES_MAJOR_VERSION == 2)
#define GLFONTSTASH_IMPLEMENTATION_ES2
#else
#define GLFONTSTASH_IMPLEMENTATION
#endif

#include "fontstash.h"
#include "gl3corefontstash.h"

