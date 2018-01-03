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

/*
 * To build the implementation OKLOG_IMPLEMENTATION needs to
 * be defined in exactly one compilation unit before including this file:
 *
 * #define OKLOG_IMPLEMENTATION
 * #include "platform_wrapper/ok_log.h"
 *
 * Note that you need to have the define before the file might be included from other included files too, because the
 * include guard prevents including later with the implementation. We are not using a separate guard for the
 * implementation here to take advantage of any include guard optimizations by the compiler
 * (https://gcc.gnu.org/onlinedocs/cppinternals/Guard-Macros.html)
 */

#ifndef PLATFORM_WRAPPER_OK_LOG_H_
#define PLATFORM_WRAPPER_OK_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef OKLOG_STATIC
#define OKLOG_DEF static
#else
#define OKLOG_DEF extern
#endif

OKLOG_DEF void initLog();
OKLOG_DEF void oklog_v(const char* tag, const char* formatString, ...);
OKLOG_DEF void oklog_d(const char* tag, const char* formatString, ...);
OKLOG_DEF void oklog_i(const char* tag, const char* formatString, ...);
OKLOG_DEF void oklog_w(const char* tag, const char* formatString, ...);
OKLOG_DEF void oklog_e(const char* tag, const char* formatString, ...);
OKLOG_DEF void oklog_c(const char* tag, const char* formatString, ...);


/*
 * ----------------------------------------------------------------------------
 * Implementation starts here.
 * ----------------------------------------------------------------------------
 */

#ifdef OKLOG_IMPLEMENTATION

#include "ok_platform.h"

#include <SDL.h>
#include <stdio.h>

#define OKLOG_DEF_INTERNAL static


OKLOG_DEF_INTERNAL void oklog__stdLogOutput(void* userdata, int category, SDL_LogPriority priority, const char* message) {
    printf("%s\n", message);
    fflush(stdout);
}

OKLOG_DEF void logInit() {
    // For now using SDL logging only on android (problems with clion etc.)
#ifndef OK_PLATFORM_ANDROID
    SDL_LogSetOutputFunction(oklog__stdLogOutput, NULL);
#endif
}

OKLOG_DEF void oklog_v(const char* tag, const char* formatString, ...) {
    va_list arguments;
    va_start(arguments, formatString);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_VERBOSE, formatString, arguments);
    va_end(arguments);
}

OKLOG_DEF void oklog_d(const char* tag, const char* formatString, ...) {
    va_list arguments;
    va_start(arguments, formatString);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG, formatString, arguments);
    va_end(arguments);
}

OKLOG_DEF void oklog_i(const char* tag, const char* formatString, ...) {
    va_list arguments;
    va_start(arguments, formatString);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, formatString, arguments);
    va_end(arguments);
}

OKLOG_DEF void oklog_w(const char* tag, const char* formatString, ...) {
    va_list arguments;
    va_start(arguments, formatString);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_WARN, formatString, arguments);
    va_end(arguments);
}

OKLOG_DEF void oklog_e(const char* tag, const char* formatString, ...) {
    va_list arguments;
    va_start(arguments, formatString);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, formatString, arguments);
    va_end(arguments);
}

OKLOG_DEF void oklog_c(const char* tag, const char* formatString, ...) {
    va_list arguments;
    va_start(arguments, formatString);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_CRITICAL, formatString, arguments);
    va_end(arguments);
}

#endif /* ifdef OKLOG_IMPLEMENTATION */

#ifdef __cplusplus
}
#endif

#endif /* ifndef PLATFORM_WRAPPER_OK_LOG_H_ */
