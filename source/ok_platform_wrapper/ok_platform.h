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

#ifndef OK_PLATFORM_H_
#define OK_PLATFORM_H_

/*
 * One of the following will be defined:
 *    OK_PLATFORM_32BIT - for a 32 bit build
 *    OK_PLATFORM_64BIT - for a 64 bit build
 *
 * One of the following platforms will be defined:
 *    OK_PLATFORM_WINDOWS
 *    OK_PLATFORM_LINUX
 *    OK_PLATFORM_MAC (mostly untested)
 *    OK_PLATFORM_IOS
 *    OK_PLATFORM_ANDROID
 *    OK_PLATFORM_NACL (not tested for a while)
 *    OK_PLATFORM_EMSCRIPTEN
 *
 * Other:
 *    OK_PLATFORM_NAME - a textual name for the platform
 *    OK_PLATFORM_HAS_THREADS - Defined if threads are supported. For example emscripten doesn't support threads.
 *
 *
 * Tested with:
 *    GCC 32bit/64bit
 *    Clang 32bit/64bit (android & ios)
 *    WIN32/WIN64 with mingw
 *    WIN32/WIN64 with visual studio
 *
 *    If you are getting "Unsupported build environment" error when compiling
 *    See: http://nadeausoftware.com/articles/2012/02/c_c_tip_how_detect_processor_type_using_compiler_predefined_macros
 *    And add the appropriate ifdefs to determine if the architecture is 32 or 64 bit.
 */


#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif


#if _WIN32 || _WIN64
#if _WIN64
#define OK_PLATFORM_64BIT
#else
#define OK_PLATFORM_32BIT
#endif

#elif __GNUC__ || __clang__
#if __x86_64__ || __aarch64__ || __powerpc64__
#define OK_PLATFORM_64BIT
#elif __i386 || __arm__ || __powerpc__
#define OK_PLATFORM_32BIT
#elif defined(EMSCRIPTEN)
/* NOTE: Just saying emscripten is 32 bit here. No idea what it actually is. */
#define OK_PLATFORM_32BIT
#endif
#endif

#if !defined(OK_PLATFORM_64BIT) && !defined(OK_PLATFORM_32BIT)
#error "Unsupported build environment"
#endif


#if defined(__ANDROID__)
#define OK_PLATFORM_ANDROID
#define OK_PLATFORM_NAME "Android"
#define OK_PLATFORM_HAS_THREADS

#elif defined(__native_client__)
#define OK_PLATFORM_NACL
#define OK_PLATFORM_NAME "NaCl"
#define OK_PLATFORM_HAS_THREADS

#elif defined(EMSCRIPTEN)
#define OK_PLATFORM_EMSCRIPTEN
#define OK_PLATFORM_NAME "Emscripten"
/* emscripten doesn't support threading yet (there is experimental support that could be enabled and tested)). */

#elif defined(_WIN32) || defined(_WIN64)
#define OK_PLATFORM_WINDOWS
#define OK_PLATFORM_NAME "Windows"
#define OK_PLATFORM_HAS_THREADS

#elif TARGET_OS_IPHONE
#define OK_PLATFORM_IOS
#define OK_PLATFORM_NAME "iOS"

#elif TARGET_OS_MAC
#define OK_PLATFORM_MAC
#define OK_PLATFORM_NAME "macOS"

/* Note that at least android also android defines __linux__, so the elif order matters. */
#elif defined(__linux__)
#define OK_PLATFORM_LINUX
#define OK_PLATFORM_NAME "Linux"

#else
#define OK_PLATFORM_NAME "Undefined"
#error "Unsupported platform"
#endif




#if defined(OK_PLATFORM_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#if defined(OK_PLATFORM_EMSCRIPTEN)
#include <emscripten.h>
#endif


#endif /* ifndef OK_PLATFORM_H_ */
