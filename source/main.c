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
// Using this separate compilation unit for header only library implementations
// to prevent polluting the app with all the implementation symbols.
//

#define OKLOG_IMPLEMENTATION
#define OKAPP_IMPLEMENTATION
#define OKGL_HELPER_IMPLEMENTATION

// NOTE: if you include the logging library before the others they will log too.
#include "okwrapper/oklog.h"
#include "okwrapper/okapp.h"
#include "okwrapper/okgl_helper.h"


//
// Fontstash.
//
#define FONTSTASH_IMPLEMENTATION

#if (OKGL_OPENGL_ES_MAJOR_VERSION == 2)
#define GLFONTSTASH_IMPLEMENTATION_ES2
#else
#define GLFONTSTASH_IMPLEMENTATION
#endif

#include "fontstash.h"
#include "gl3corefontstash.h"

