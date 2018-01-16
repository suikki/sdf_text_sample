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

#ifndef OKWRAPPER_OKPLATFORM_H_
#define OKWRAPPER_OKPLATFORM_H_

//
// Header with definitions to help detecting the target platform and to help
// identify some platform properties.
//
//
//  One of the following will be defined as 1:
//     OKPLATFORM_32BIT - for a 32 bit target platform
//     OKPLATFORM_64BIT - for a 64 bit target platform
//
//  One of the following platforms will be defined as 1:
//     OKPLATFORM_WINDOWS
//     OKPLATFORM_LINUX
//     OKPLATFORM_MAC
//     OKPLATFORM_IOS
//     OKPLATFORM_ANDROID
//     OKPLATFORM_EMSCRIPTEN
//
//  Other:
//     OKPLATFORM_NAME - a textual name for the platform
//     OKPLATFORM_HAS_THREADS - Defined as 1 if threads are supported 0 if not.
//         For example emscripten doesn't support threads yet.
//
//
//
//  Note that instead of being undefined all the defines have value 0 to help
//  entering them using an IDE that can autocomplete. Platform specific code
//  can be written like this:
//
//    #if OKPLATFORM_WINDOWS
//        <windows specific code>
//    #elif OKPLATFORM_ANDROID
//        <android specific code>
//    #else
//        <other platforms>
//    #endif
//
//
//  Tested with:
//     GCC 32bit/64bit
//     Clang 32bit/64bit (android & ios)
//     WIN32/WIN64 with mingw
//     WIN32/WIN64 with visual studio
//
//  If you are getting "Unsupported build environment" error when compiling See:
//  http://nadeausoftware.com/articles/2012/02/c_c_tip_how_detect_processor_type_using_compiler_predefined_macros
//  and add the appropriate defines to determine if the architecture is 32 or
//  64 bit.
//


#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif


#if _WIN32 || _WIN64
#if _WIN64
#define OKPLATFORM_32BIT 0
#define OKPLATFORM_64BIT 1
#else
#define OKPLATFORM_32BIT 1
#define OKPLATFORM_64BIT 0
#endif

#elif __GNUC__ || __clang__
#if __x86_64__ || __aarch64__ || __powerpc64__
#define OKPLATFORM_32BIT 0
#define OKPLATFORM_64BIT 1
#elif __i386 || __arm__ || __powerpc__
#define OKPLATFORM_32BIT 1
#define OKPLATFORM_64BIT 0
#elif defined(EMSCRIPTEN)
// NOTE: Just defining emscripten as 32 bit system here. No idea what it actually should be.
#define OKPLATFORM_32BIT 1
#define OKPLATFORM_64BIT 0
#endif
#endif

#if !defined(OKPLATFORM_64BIT) && !defined(OKPLATFORM_32BIT)
#error "Unsupported build environment"
#endif


// Default values.
#define OKPLATFORM_ANDROID 0
#define OKPLATFORM_EMSCRIPTEN 0
#define OKPLATFORM_WINDOWS 0
#define OKPLATFORM_IOS 0
#define OKPLATFORM_MAC 0
#define OKPLATFORM_LINUX 0


#if defined(__ANDROID__)
#undef OKPLATFORM_ANDROID
#define OKPLATFORM_ANDROID 1
#define OKPLATFORM_NAME "Android"
#define OKPLATFORM_HAS_THREADS 1

#elif defined(EMSCRIPTEN)
#undef OKPLATFORM_EMSCRIPTEN
#define OKPLATFORM_EMSCRIPTEN 1
#define OKPLATFORM_NAME "Emscripten"
// Emscripten doesn't support threading yet (there is experimental support that could be enabled and tested)).

#elif defined(_WIN32) || defined(_WIN64)
#undef OKPLATFORM_WINDOWS
#define OKPLATFORM_WINDOWS 1
#define OKPLATFORM_NAME "Windows"
#define OKPLATFORM_HAS_THREADS

#elif TARGET_OS_IPHONE
#undef OKPLATFORM_IOS
#define OKPLATFORM_IOS 1
#define OKPLATFORM_NAME "iOS"

#elif TARGET_OS_MAC
#undef OKPLATFORM_MAC
#define OKPLATFORM_MAC 1
#define OKPLATFORM_NAME "macOS"

// Note that also android defines __linux__, so the elif order matters.
#elif defined(__linux__)
#undef OKPLATFORM_LINUX
#define OKPLATFORM_LINUX 1
#define OKPLATFORM_NAME "Linux"

#else
#error "Unsupported platform"
#endif


#if OKPLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#if OKPLATFORM_EMSCRIPTEN
#include <emscripten.h>
#endif


#endif // ifndef OKWRAPPER_OKPLATFORM_H_
