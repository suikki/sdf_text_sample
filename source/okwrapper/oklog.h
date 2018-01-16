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
// Do this:
//   #define OKLOG_IMPLEMENTATION
// before you include this file in *one* C or C++ file to create the
// implementation.
//
// i.e. it should look like this:
// #define OKLOG_IMPLEMENTATION
// #include "okwrapper/oklog.h"
//
//
// This is a header only crossplatform logging library.
//
// Note: Verbose and Debug log calls are completely removed from non debug
// builds in preprocessing by default. Make sure calls to them don't do any
// actual work. Only the calls using the log  macros are removed. Any verbose
// messages using a direct call to the oklog_log function is not be removed.
//
// Note that you need to have the define before the file might be included from
// other included files too, because the include guard prevents including later
// with the implementation. We are not using a separate guard for the
// implementation here to take advantage of any include guard optimizations by
// the compiler (https://gcc.gnu.org/onlinedocs/cppinternals/Guard-Macros.html).
// (Maybe it's not worth it and should be changed).
//
//
// # Revision history
//      0.1   (2018-01-10) Not officially released.
//
//
// # Todo list
//   - allow defining custom colors for log fields
//   - check that log parameters are valid (on debug builds)
//   - aligning text in log fields
//   - iOS/mac logging
//   - support printing to OutputDebugString on windows
//   - file logging
//   - utf-8 causes problems with padding
//   - multithreading support
//   - add a define to disable color support entirely
//

#ifndef OKWRAPPER_OKLOG_H_
#define OKWRAPPER_OKLOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef OKLOG_STATIC
#define OKIMPLLOG_DEF static
#else
#define OKIMPLLOG_DEF extern
#endif

typedef enum OKLOG_LogLevel {
    OKLOG_LEVEL_ALL = 0, // Only for filtering.

    OKLOG_LEVEL_VERBOSE, // Use these in the actual logging calls.
    OKLOG_LEVEL_DEBUG,
    OKLOG_LEVEL_INFO,
    OKLOG_LEVEL_WARNING,
    OKLOG_LEVEL_ERROR,
    OKLOG_LEVEL_FATAL,

    OKLOG_LEVEL_SILENT, // Only for filtering.
} OKLOG_LogLevel;

typedef enum OKLOG_ColorMode {
    OKLOG_MODE_COLOR_AUTO = 0,
    OKLOG_MODE_COLOR_NONE,
    OKLOG_MODE_COLOR_ANSI,
    OKLOG_MODE_COLOR_ANSI256,
} OKLOG_ColorMode;

typedef enum OKLOG_Color {
    OKLOG_COLOR_NONE = -1,
    OKLOG_COLOR_BLACK = 0,
    OKLOG_COLOR_RED,
    OKLOG_COLOR_GREEN,
    OKLOG_COLOR_YELLOW,
    OKLOG_COLOR_BLUE,
    OKLOG_COLOR_MAGENTA,
    OKLOG_COLOR_CYAN,
    OKLOG_COLOR_WHITE,
    OKLOG_COLOR_BRIGHT_BLACK,
    OKLOG_COLOR_BRIGHT_RED,
    OKLOG_COLOR_BRIGHT_GREEN,
    OKLOG_COLOR_BRIGHT_YELLOW,
    OKLOG_COLOR_BRIGHT_BLUE,
    OKLOG_COLOR_BRIGHT_MAGENTA,
    OKLOG_COLOR_BRIGHT_CYAN,
    OKLOG_COLOR_BRIGHT_WHITE,
} OKLOG_Color;

typedef struct OKLOG_ColorCode {
    char data[12];
} OKLOG_ColorCode;


// Define some shorthands. Can be disabled if they cause namespace issues.
#ifndef OKLOG_NO_SHORTHANDS
#define log_v(tag, ...) oklog_verbose(tag, __VA_ARGS__)
#define log_d(tag, ...) oklog_debug(tag, __VA_ARGS__)
#define log_i(tag, ...) oklog_info(tag, __VA_ARGS__)
#define log_w(tag, ...) oklog_warning(tag, __VA_ARGS__)
#define log_e(tag, ...) oklog_error(tag, __VA_ARGS__)
#define log_f(tag, ...) oklog_fatal(tag, __VA_ARGS__)
#endif

//
// Setup which data is stripped away from release builds.
//
#if defined(OKLOG_NO_RELEASE_STRIPPING) || defined(DEBUG)
// The debug build includes all levels and passes the file & line info.
#define OKIMPLLOG_ENABLE_VERBOSE 1
#define OKIMPLLOG_ENABLE_DEBUG 1
#define OKIMPLLOG_ENABLE_INFO 1
#define OKIMPLLOG_ENABLE_WARNING 1
#define OKIMPLLOG_ENABLE_ERROR 1
#define OKIMPLLOG_ENABLE_FATAL 1
#define OKIMPLLOG_FILE __FILE__
#define OKIMPLLOG_LINE __LINE__
#else
// In release mode don't include verbose and debug code or file & line info.
#define OKIMPLLOG_ENABLE_VERBOSE 0
#define OKIMPLLOG_ENABLE_DEBUG 0
#define OKIMPLLOG_ENABLE_INFO 1
#define OKIMPLLOG_ENABLE_WARNING 1
#define OKIMPLLOG_ENABLE_ERROR 1
#define OKIMPLLOG_ENABLE_FATAL 1
#define OKIMPLLOG_FILE 0
#define OKIMPLLOG_LINE 0
#endif

// Used to strip away logging calls on build time.
#define OKIMPLLOG_CONDITIONAL(condition, func) do { if (condition) (func); } while (0)

#define oklog_verbose(tag, ...) OKIMPLLOG_CONDITIONAL(OKIMPLLOG_ENABLE_VERBOSE, oklog_log(OKLOG_LEVEL_VERBOSE, tag, OKIMPLLOG_FILE, OKIMPLLOG_LINE, __VA_ARGS__))
#define oklog_debug(tag, ...) OKIMPLLOG_CONDITIONAL(OKIMPLLOG_ENABLE_DEBUG, oklog_log(OKLOG_LEVEL_DEBUG, tag, OKIMPLLOG_FILE, OKIMPLLOG_LINE, __VA_ARGS__))
#define oklog_info(tag, ...) OKIMPLLOG_CONDITIONAL(OKIMPLLOG_ENABLE_INFO, oklog_log(OKLOG_LEVEL_INFO, tag, OKIMPLLOG_FILE, OKIMPLLOG_LINE, __VA_ARGS__))
#define oklog_warning(tag, ...) OKIMPLLOG_CONDITIONAL(OKIMPLLOG_ENABLE_WARNING, oklog_log(OKLOG_LEVEL_WARNING, tag, OKIMPLLOG_FILE, OKIMPLLOG_LINE, __VA_ARGS__))
#define oklog_error(tag, ...) OKIMPLLOG_CONDITIONAL(OKIMPLLOG_ENABLE_ERROR, oklog_log(OKLOG_LEVEL_ERROR, tag, OKIMPLLOG_FILE, OKIMPLLOG_LINE, __VA_ARGS__))
#define oklog_fatal(tag, ...) OKIMPLLOG_CONDITIONAL(OKIMPLLOG_ENABLE_FATAL, oklog_log(OKLOG_LEVEL_FATAL, tag, OKIMPLLOG_FILE, OKIMPLLOG_LINE, __VA_ARGS__))


OKIMPLLOG_DEF void oklog_log(OKLOG_LogLevel logLevel, const char* tag, const char* file, int line, const char* formatString, ...);

OKIMPLLOG_DEF void oklog_setFormat(const char* logFormatting);

OKIMPLLOG_DEF void oklog_setLogLevelFilter(OKLOG_LogLevel logLevelFilter);
OKIMPLLOG_DEF OKLOG_LogLevel oklog_getLogLevelFilter();
OKIMPLLOG_DEF void oklog_setFilterCallback(int (* filterCallback)(OKLOG_LogLevel logLevel, const char* tag));

OKIMPLLOG_DEF const char* oklog_getLogLevelName(OKLOG_LogLevel logLevel);
OKIMPLLOG_DEF const char* oklog_getLogLevelNameShort(OKLOG_LogLevel logLevel);

//
// Helper function for getting logging options directly from command line parameters.
// This checks for:
//  --color          forces colored output
//  --color=none     forces uncolored output
//  --color=auto     use colors only if the output is a terminal and output is not redirected to a file or pipe
//  --color=ansi     same as using just --color)
//  --color=ansi256  nicer and more consistent colors (where supported)
//
OKIMPLLOG_DEF void oklog_parseCommandLineOptions(int argc, char* argv[]);

//
// Color support.
//
OKIMPLLOG_DEF void oklog_setColorMode(OKLOG_ColorMode colorMode);
OKIMPLLOG_DEF OKLOG_ColorMode oklog_getColorMode();

// See https://en.wikipedia.org/wiki/ANSI_escape_code for color values.
OKIMPLLOG_DEF int oklog_foregroundColor(OKLOG_ColorCode* colorCode, OKLOG_Color color, int color256);
OKIMPLLOG_DEF int oklog_backgroundColor(OKLOG_ColorCode* colorCode, OKLOG_Color color, int color256);
OKIMPLLOG_DEF const char* oklog_getResetColorCode();

// TODO: figure out a better API and better names
OKIMPLLOG_DEF void oklog_setLabelColor(OKLOG_LogLevel logLevel, OKLOG_Color fgColor, int fgColor256, OKLOG_Color bgColor, int bgColor256);
OKIMPLLOG_DEF void oklog_setMessageColor(OKLOG_LogLevel logLevel, OKLOG_Color fgColor, int fgColor256, OKLOG_Color bgColor, int bgColor256);

// TODO: setting color for log fields
//OKIMPLLOG_DEF void oklog_setFieldColorNone(const char* fieldId);
//OKIMPLLOG_DEF void oklog_setFieldColorLabel(const char* fieldId);
//OKIMPLLOG_DEF void oklog_setFieldColorMessage(const char* fieldId);
//OKIMPLLOG_DEF void oklog_setFieldColor(const char* fieldId, OKLOG_Color fgColor, int fgColor256, OKLOG_Color bgColor, int bgColor256);



// ----------------------------------------------------------------------------
// Implementation starts here.
// ----------------------------------------------------------------------------
#ifdef OKLOG_IMPLEMENTATION

#if defined(__ANDROID__)
#include <android/log.h>
#endif

#if _WIN32 || _WIN64
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#endif

#if !(_WIN32 || _WIN64) && !defined(__ANDROID__)
#include <unistd.h>
#endif


#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

//
// Internal datastructures and state.
//

#define OKIMPLLOG_DEF_INTERNAL static
#define OKIMPLLOG_DATA_INTERNAL static

#ifndef OKLOG_MAX_FIELD_COUNT
#define OKLOG_MAX_FIELD_COUNT 10
#endif

#ifndef OKIMPLLOG_DEFAULT_ANSI_COLOR_MODE
#define OKIMPLLOG_DEFAULT_ANSI_COLOR_MODE OKLOG_MODE_COLOR_ANSI256
#endif

#ifndef OKIMPLLOG_DEFAULT_FORMAT
#define OKIMPLLOG_DEFAULT_FORMAT "{time_short} {level_short} {tag} {file_short} {message}"
#endif

#ifndef OKIMPLLOG_DEFAULT_FORMAT_STRIPPED
#define OKIMPLLOG_DEFAULT_FORMAT_STRIPPED "{time_short} {level_short} {message}"
#endif

#define OKIMPLLOG_NUM_LOG_LEVELS OKLOG_LEVEL_SILENT
#define OKIMPLLOG_COLOR_CODE_SIZE sizeof("\x1B[38;5;255m")

#define OKIMPLLOG_FG_INDEX_ANSI 0
#define OKIMPLLOG_FG_INDEX_ANSI256 1
#define OKIMPLLOG_BG_INDEX_ANSI 2
#define OKIMPLLOG_BG_INDEX_ANSI256 3

typedef enum OKIMPLLOG_ColorType {
    OKIMPLLOG_TYPE_COLOR_NONE = 0,
    OKIMPLLOG_TYPE_COLOR_CODE,
    OKIMPLLOG_TYPE_COLOR_LOG_LEVEL_LABEL_COLOR,
    OKIMPLLOG_TYPE_COLOR_LOG_LEVEL_MESSAGE_COLOR,
} OKIMPLLOG_ColorType;

typedef enum OKIMPLLOG_FieldType {
    OKIMPLLOG_LOG_FIELD_NONE = 0,
    OKIMPLLOG_LOG_FIELD_TIME,
    OKIMPLLOG_LOG_FIELD_LOG_LEVEL,
    OKIMPLLOG_LOG_FIELD_LOG_LEVEL_SHORT,
    OKIMPLLOG_LOG_FIELD_FILE,
    OKIMPLLOG_LOG_FIELD_FILE_SHORT,
    OKIMPLLOG_LOG_FIELD_TAG,
    OKIMPLLOG_LOG_FIELD_MESSAGE,
} OKIMPLLOG_FieldType;

typedef struct OKIMPLLOG_LogField {
    OKIMPLLOG_FieldType type;
    const char* formatString;
    int startPadding;
    char padEnd[6];

    int colorType;
    int color[4];
    // Cached color code for the current color mode.
    char colorCode[OKIMPLLOG_COLOR_CODE_SIZE * 2 - 1];
} OKIMPLLOG_LogField;

typedef struct OKIMPLLOG_context {
    OKLOG_LogLevel logLevelFilter;
    OKLOG_ColorMode colorMode;
    int (* filterCallback)(OKLOG_LogLevel logLevel, const char* tag);
    int initDone;
    OKIMPLLOG_LogField fields[OKLOG_MAX_FIELD_COUNT];
    int fieldCount;

    // For each logging level:
    //   0 = fg ansi color index (OKIMPLLOG_FG_INDEX_ANSI)
    //   1 = fg ansi256 color index (OKIMPLLOG_FG_INDEX_ANSI256)
    //   2 = bg ansi color index (OKIMPLLOG_BG_INDEX_ANSI)
    //   3 = bg ansi256 color index (OKIMPLLOG_BG_INDEX_ANSI256)
    // -1 to indicate no color definition.
    int labelColors[OKIMPLLOG_NUM_LOG_LEVELS][4];
    int messageColors[OKIMPLLOG_NUM_LOG_LEVELS][4];

    // Cached color mode strings for each logging level using the current color mode.
    // Combining possible fg and bg colors to one string.
    char labelColorCode[OKIMPLLOG_NUM_LOG_LEVELS][OKIMPLLOG_COLOR_CODE_SIZE * 2 - 1];
    char messageColorCode[OKIMPLLOG_NUM_LOG_LEVELS][OKIMPLLOG_COLOR_CODE_SIZE * 2 - 1];
} OKIMPLLOG_context;

// NOTE: These must match the OKLOG_LogLevel enum.
OKIMPLLOG_DATA_INTERNAL const char* OKIMPLLOG_LOG_LEVEL_NAMES[] = {
        "ALL", "VERBOSE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL", "SILENT",
};

// NOTE: These must match the OKLOG_LogLevel enum.
OKIMPLLOG_DATA_INTERNAL const char* OKIMPLLOG_LOG_LEVEL_NAMES_SHORT[] = {
        "A", "V", "D", "I", "W", "E", "F", "S",
};


// TODO: What if no global non-const data is allowed? Passing a context in every log call seems cumbersome.
OKIMPLLOG_DATA_INTERNAL OKIMPLLOG_context okimpllog_context = {.logLevelFilter = OKLOG_LEVEL_ALL, .colorMode = OKLOG_MODE_COLOR_NONE};


//
// Internal helpers.
//

OKIMPLLOG_DEF_INTERNAL void okimpllog_confirmInit();
OKIMPLLOG_DEF_INTERNAL int okimpllog_checkLogFilter(OKLOG_LogLevel logLevel, const char* tag);
OKIMPLLOG_DEF_INTERNAL OKIMPLLOG_LogField* okimpllog_addField();
OKIMPLLOG_DEF_INTERNAL const char* okimpllog_getFilename(const char* file, int fullpath);
OKIMPLLOG_DEF_INTERNAL int okimpllog_appendString(char* text, int textLength, size_t bufferSize, const char* value);
OKIMPLLOG_DEF_INTERNAL int okimpllog_appendInt(char* text, int textLength, size_t bufferSize, int value);
OKIMPLLOG_DEF_INTERNAL int okimpllog_generateColorCodeAnsi(char* buffer, size_t bufferSize, int fgColor, int bgColor);
OKIMPLLOG_DEF_INTERNAL int okimpllog_generateColorCodeAnsi256(char* buffer, size_t bufferSize, int fgColor, int bgColor);
OKIMPLLOG_DEF_INTERNAL int okimpllog_generateColorCode(char* buffer, size_t bufferSize, int background, int ansi, int ansi256);
OKIMPLLOG_DEF_INTERNAL void okimpllog_updateFieldColorCache();
OKIMPLLOG_DEF_INTERNAL void okimpllog_updateLogLevelColorCache();

//
// API function implementations.
//

OKIMPLLOG_DEF void oklog_setLogLevelFilter(OKLOG_LogLevel logLevelFilter) {
    okimpllog_confirmInit();

    // Check that the level is valid.
    if (logLevelFilter > OKLOG_LEVEL_SILENT) {
        logLevelFilter = OKLOG_LEVEL_SILENT;
    } else if (logLevelFilter < OKLOG_LEVEL_ALL) {
        logLevelFilter = OKLOG_LEVEL_ALL;
    }

    okimpllog_context.logLevelFilter = logLevelFilter;
}

OKIMPLLOG_DEF OKLOG_LogLevel oklog_getLogLevelFilter() {
    okimpllog_confirmInit();
    return okimpllog_context.logLevelFilter;
}

OKIMPLLOG_DEF void oklog_setFilterCallback(int (* filterCallback)(OKLOG_LogLevel, const char*)) {
    okimpllog_confirmInit();
    okimpllog_context.filterCallback = filterCallback;
}

OKIMPLLOG_DEF void oklog_setFormat(const char* logFormatting) {
    okimpllog_confirmInit();

    //
    // Just a very simple stupid parser to avoid dependencies.
    //

    okimpllog_context.fieldCount = 0;
    OKIMPLLOG_LogField* field = 0;
    const char* current = logFormatting;
    while (*current != 0) {
        if (*current != '{') {
            // Add an empty field just to for the plain text padding.
            if ((field = okimpllog_addField()) != 0) {
                field->type = OKIMPLLOG_LOG_FIELD_NONE;
                field->formatString = 0;
            }
        } else {
            // Start of parameter.
            current++;
            const char* nameStart = current;
            while (*current != 0) {
                if (*current == '}') {
                    size_t nameLength = current - nameStart;

                    if (*nameStart >= '0' && *nameStart <= '9') {
                        // This is a field that just add some padding for alignment.
                        char* end = (char*) current;
                        int padding = strtol(nameStart, &end, 10);
                        if (padding > 0 && (field = okimpllog_addField()) != 0) {
                            field->type = OKIMPLLOG_LOG_FIELD_NONE;
                            field->formatString = 0;
                            field->startPadding = padding;
                        }

                    } else if (memcmp(nameStart, "message", nameLength) == 0) {
                        if ((field = okimpllog_addField()) != 0) {
                            field->type = OKIMPLLOG_LOG_FIELD_MESSAGE;
                            field->formatString = 0; // Not used.
                            field->colorType = OKIMPLLOG_TYPE_COLOR_LOG_LEVEL_MESSAGE_COLOR;
                        }

                    } else if (memcmp(nameStart, "time", nameLength) == 0) {
                        if ((field = okimpllog_addField()) != 0) {
                            field->type = OKIMPLLOG_LOG_FIELD_TIME;
                            field->formatString = "%Y-%m-%d %H:%M:%S";
                        }

                    } else if (memcmp(nameStart, "time_short", nameLength) == 0) {
                        if ((field = okimpllog_addField()) != 0) {
                            field->type = OKIMPLLOG_LOG_FIELD_TIME;
                            field->formatString = "%H:%M:%S";
                        }

                    } else if (memcmp(nameStart, "level", nameLength) == 0) {
                        if ((field = okimpllog_addField()) != 0) {
                            field->type = OKIMPLLOG_LOG_FIELD_LOG_LEVEL;
                            field->formatString = "%s";
                            field->colorType = OKIMPLLOG_TYPE_COLOR_LOG_LEVEL_LABEL_COLOR;
                        }

                    } else if (memcmp(nameStart, "level_short", nameLength) == 0) {
                        if ((field = okimpllog_addField()) != 0) {
                            field->type = OKIMPLLOG_LOG_FIELD_LOG_LEVEL_SHORT;
                            field->formatString = "%s";
                            field->colorType = OKIMPLLOG_TYPE_COLOR_LOG_LEVEL_LABEL_COLOR;
                        }

                    } else if (memcmp(nameStart, "tag", nameLength) == 0) {
                        if ((field = okimpllog_addField()) != 0) {
                            field->type = OKIMPLLOG_LOG_FIELD_TAG;
                            field->formatString = "%s";
                            field->colorType = OKIMPLLOG_TYPE_COLOR_CODE;
                            field->color[OKIMPLLOG_FG_INDEX_ANSI] = OKLOG_COLOR_CYAN;
                            field->color[OKIMPLLOG_FG_INDEX_ANSI256] = 30;
                            field->color[OKIMPLLOG_BG_INDEX_ANSI] = -1;
                            field->color[OKIMPLLOG_BG_INDEX_ANSI256] = -1;
                        }

                    } else if (memcmp(nameStart, "file", nameLength) == 0) {
                        if ((field = okimpllog_addField()) != 0) {
                            field->type = OKIMPLLOG_LOG_FIELD_FILE;
                            field->formatString = "%s:%d";
                            field->colorType = OKIMPLLOG_TYPE_COLOR_CODE;
                            field->color[OKIMPLLOG_FG_INDEX_ANSI] = OKLOG_COLOR_BRIGHT_BLUE;
                            field->color[OKIMPLLOG_FG_INDEX_ANSI256] = 68;
                            field->color[OKIMPLLOG_BG_INDEX_ANSI] = -1;
                            field->color[OKIMPLLOG_BG_INDEX_ANSI256] = -1;
                        }

                    } else if (memcmp(nameStart, "file_short", nameLength) == 0) {
                        if ((field = okimpllog_addField()) != 0) {
                            field->type = OKIMPLLOG_LOG_FIELD_FILE_SHORT;
                            field->formatString = "%s:%d";
                            field->colorType = OKIMPLLOG_TYPE_COLOR_CODE;
                            field->color[OKIMPLLOG_FG_INDEX_ANSI] = OKLOG_COLOR_BRIGHT_BLUE;
                            field->color[OKIMPLLOG_FG_INDEX_ANSI256] = 68;
                            field->color[OKIMPLLOG_BG_INDEX_ANSI] = -1;
                            field->color[OKIMPLLOG_BG_INDEX_ANSI256] = -1;
                        }
                    }

                    current++;
                    break;
                }
                current++;
            }
        }

        // Parse text data between the fields (we have only a few bytes reserved for them, rest are discarded).
        if (field) {
            int maxPad = sizeof(field->padEnd) - 1;
            char* padEnd = field->padEnd;
            while ((*current != 0 && maxPad > 0) && *current != '{') {
                *padEnd++ = *current;
                current++;
                maxPad--;
            }
            current--;
        }

        current++;
    }

    okimpllog_updateFieldColorCache();
}

OKIMPLLOG_DEF const char* oklog_getLogLevelName(OKLOG_LogLevel logLevel) {
    return OKIMPLLOG_LOG_LEVEL_NAMES[(int) logLevel];
}

OKIMPLLOG_DEF const char* oklog_getLogLevelNameShort(OKLOG_LogLevel logLevel) {
    return OKIMPLLOG_LOG_LEVEL_NAMES_SHORT[(int) logLevel];
}

#if defined(__ANDROID__)

OKIMPLLOG_DEF void oklog_log(OKLOG_LogLevel logLevel, const char* tag, const char* file, int line, const char* formatString, ...) {

    if (!okimpllog_checkLogFilter(logLevel, tag)) {
        return;
    }

    okimpllog_confirmInit();

    int androidPriority = 0;
    switch (logLevel) {
        case OKLOG_LEVEL_VERBOSE:
            androidPriority = ANDROID_LOG_VERBOSE;
            break;
        case OKLOG_LEVEL_DEBUG:
            androidPriority = ANDROID_LOG_DEBUG;
            break;
        case OKLOG_LEVEL_INFO:
            androidPriority = ANDROID_LOG_INFO;
            break;
        case OKLOG_LEVEL_WARNING:
            androidPriority = ANDROID_LOG_WARN;
            break;
        case OKLOG_LEVEL_ERROR:
            androidPriority = ANDROID_LOG_ERROR;
            break;
        case OKLOG_LEVEL_FATAL:
            androidPriority = ANDROID_LOG_FATAL;
            break;
        default:
            androidPriority = ANDROID_LOG_DEFAULT;
    }

    const char* androidTag;
    if (tag) {
        androidTag = tag;
    } else if (file) {
        androidTag = okimpllog_getFilename(file, 0);
    } else {
        androidTag = "app";
    }

    va_list arguments;
    va_start(arguments, formatString);
    __android_log_vprint(androidPriority, androidTag, formatString, arguments);
    va_end(arguments);
}

#elif defined(OKLOG_IMPLEMENTATION_OUTPUT_SDL)

#include <SDL.h>

OKIMPLLOG_DEF void oklog_log(OKLOG_LogLevel logLevel, const char* tag, const char* file, int line, const char* formatString, ...) {
    if (!okimpllog_checkLogFilter(logLevel, tag)) {
        return;
    }

    okimpllog_confirmInit();

    SDL_LogPriority sdlPriority;
    switch (logLevel) {
        case OKLOG_LEVEL_VERBOSE:
            sdlPriority = SDL_LOG_PRIORITY_VERBOSE;
            break;
        case OKLOG_LEVEL_DEBUG:
            sdlPriority = SDL_LOG_PRIORITY_DEBUG;
            break;
        case OKLOG_LEVEL_INFO:
            sdlPriority = SDL_LOG_PRIORITY_INFO;
            break;
        case OKLOG_LEVEL_WARNING:
            sdlPriority = SDL_LOG_PRIORITY_WARN;
            break;
        case OKLOG_LEVEL_ERROR:
            sdlPriority = SDL_LOG_PRIORITY_ERROR;
            break;
        case OKLOG_LEVEL_FATAL:
            sdlPriority = SDL_LOG_PRIORITY_CRITICAL;
            break;
        default:
            sdlPriority = SDL_LOG_PRIORITY_INFO;
    }

    va_list arguments;
    va_start(arguments, formatString);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, sdlPriority, formatString, arguments);
    va_end(arguments);
}

#else

OKIMPLLOG_DEF void oklog_log(OKLOG_LogLevel logLevel, const char* tag, const char* file, int line, const char* formatString, ...) {

    if (!okimpllog_checkLogFilter(logLevel, tag)) {
        return;
    }

    okimpllog_confirmInit();

    int charsPrinted = 0;
    for (int i = 0; i < okimpllog_context.fieldCount; ++i) {
        OKIMPLLOG_LogField* field = &okimpllog_context.fields[i];

        if ((field->startPadding > 0) && (charsPrinted < field->startPadding)) {
            const int padding = field->startPadding - charsPrinted;
            charsPrinted += printf("%*c", padding, ' ');
        }

        const char* colorCode = "";
        switch (field->colorType) {
            case OKIMPLLOG_TYPE_COLOR_CODE:
                colorCode = field->colorCode;
                break;

            case OKIMPLLOG_TYPE_COLOR_LOG_LEVEL_LABEL_COLOR:
                colorCode = okimpllog_context.labelColorCode[logLevel];
                break;

            case OKIMPLLOG_TYPE_COLOR_LOG_LEVEL_MESSAGE_COLOR:
                colorCode = okimpllog_context.messageColorCode[logLevel];
                break;

            default:
                break;
        }
        if (*colorCode != '\0') {
            printf("%s", colorCode);
        }

        switch (field->type) {
            case OKIMPLLOG_LOG_FIELD_TIME: {
                time_t t = time(0);
                struct tm* lt = localtime(&t);
                char timeBuffer[30];
                const size_t bufferSize = sizeof(timeBuffer);
                size_t timeChars = strftime(timeBuffer, bufferSize, field->formatString, lt);
                if (timeChars > 0 && timeChars < bufferSize) {
                    // Only print the time if there was enough space for the whole time.
                    timeBuffer[bufferSize - 1] = '\0';
                    charsPrinted += printf("%s", timeBuffer);
                }
                break;
            }

            case OKIMPLLOG_LOG_FIELD_LOG_LEVEL: {
                charsPrinted += printf(field->formatString, oklog_getLogLevelName(logLevel));
                break;
            }

            case OKIMPLLOG_LOG_FIELD_LOG_LEVEL_SHORT: {
                charsPrinted += printf(field->formatString, oklog_getLogLevelNameShort(logLevel));
                break;
            }

            case OKIMPLLOG_LOG_FIELD_TAG: {
                if (tag) {
                    charsPrinted += printf(field->formatString, tag);
                }
                break;
            }

            case OKIMPLLOG_LOG_FIELD_FILE: {
                if (file) {
                    charsPrinted += printf(field->formatString, okimpllog_getFilename(file, 1), line);
                }
                break;
            }

            case OKIMPLLOG_LOG_FIELD_FILE_SHORT: {
                if (file) {
                    charsPrinted += printf(field->formatString, okimpllog_getFilename(file, 0), line);
                }
                break;
            }

            case OKIMPLLOG_LOG_FIELD_MESSAGE: {
                va_list arguments;
                va_start(arguments, formatString);
                charsPrinted += vprintf(formatString, arguments);
                va_end(arguments);
                break;
            }

            default:
                break;
        }

        if (*colorCode != '\0') {
            printf("%s", oklog_getResetColorCode());
        }

        if (*field->padEnd != '\0') {
            charsPrinted += printf("%s", field->padEnd);
        }
    }

    if (formatString && formatString[0] != '0') {
        size_t messageChars = strlen(formatString);
        if (formatString[messageChars - 1] != '\n') {
            // if the last character is not "new line" add one.
            printf("\n");
        }
    }
    fflush(stdout);
}
#endif

OKIMPLLOG_DEF void oklog_parseCommandLineOptions(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        const char colorParam[] = "--color";
        int colorcmp = strcmp(argv[1], colorParam);
        if (colorcmp == 0) {
            oklog_setColorMode(OKLOG_MODE_COLOR_ANSI);
        } else if (colorcmp > 0) {
            const char* value = argv[i] + sizeof(colorParam) - 1;
            if (strcmp(value, "=ansi") == 0) {
                oklog_setColorMode(OKLOG_MODE_COLOR_ANSI);

            } else if (strcmp(value, "=auto") == 0) {
                oklog_setColorMode(OKLOG_MODE_COLOR_AUTO);

            } else if (strcmp(value, "=ansi256") == 0) {
                oklog_setColorMode(OKLOG_MODE_COLOR_ANSI256);

            } else if (strcmp(value, "=none") == 0) {
                oklog_setColorMode(OKLOG_MODE_COLOR_NONE);
            }
        }
    }
}

OKIMPLLOG_DEF void oklog_setColorMode(OKLOG_ColorMode colorMode) {
    okimpllog_confirmInit();

    OKLOG_ColorMode oldColorMode = okimpllog_context.colorMode;

    if (colorMode == OKLOG_MODE_COLOR_AUTO) {
        okimpllog_context.colorMode = OKLOG_MODE_COLOR_NONE;


#if _WIN32 || _WIN64
        const HANDLE stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        if (stdHandle) {
            // Enable support for ansi color codes if ENABLE_VIRTUAL_TERMINAL_PROCESSING is supported.
            DWORD handleMode;
            if (GetConsoleMode(stdHandle, &handleMode) == 0) {
                // Not a real console. The output is being redirected.
            } else {
                handleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                SetConsoleMode(stdHandle, handleMode);

                // TODO: not sure if this works. should test on a windows device that does not support ansi colors.
                GetConsoleMode(stdHandle, &handleMode);
                if ((handleMode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) != 0) {
                    okimpllog_context.colorMode = OKIMPLLOG_DEFAULT_ANSI_COLOR_MODE;
                }
            }
        }

#elif defined(__ANDROID__) || defined (EMSCRIPTEN)
        // Not using colors by default on android or emscripten.

#else
        if (isatty(fileno(stdout))) {
            // By default enable ansi colors if the output id not being redirected.
            okimpllog_context.colorMode = OKIMPLLOG_DEFAULT_ANSI_COLOR_MODE;
        }
#endif

    } else {
        okimpllog_context.colorMode = colorMode;
    }

    // If the color mode changed -> update cached color codes.
    if (oldColorMode != okimpllog_context.colorMode) {
        okimpllog_updateLogLevelColorCache();
        okimpllog_updateFieldColorCache();
    }
}

OKIMPLLOG_DEF OKLOG_ColorMode oklog_getColorMode() {
    okimpllog_confirmInit();
    return okimpllog_context.colorMode;
}

OKIMPLLOG_DEF int oklog_foregroundColor(OKLOG_ColorCode* colorCode, OKLOG_Color color, int color256) {
    okimpllog_confirmInit();
    return okimpllog_generateColorCode(colorCode->data, sizeof(colorCode->data), 0, color, color256);
}

OKIMPLLOG_DEF int oklog_backgroundColor(OKLOG_ColorCode* colorCode, OKLOG_Color color, int color256) {
    okimpllog_confirmInit();
    return okimpllog_generateColorCode(colorCode->data, sizeof(colorCode->data), 1, color, color256);
}

OKIMPLLOG_DEF const char* oklog_getResetColorCode() {
    return okimpllog_context.colorMode == OKLOG_MODE_COLOR_NONE ? "" : "\x1B[0m";
}

OKIMPLLOG_DEF void oklog_setLabelColor(OKLOG_LogLevel logLevel, OKLOG_Color fgColor, int fgColor256, OKLOG_Color bgColor, int bgColor256) {
    if (logLevel > OKLOG_LEVEL_ALL && logLevel < OKLOG_LEVEL_SILENT) {
        okimpllog_confirmInit();

        okimpllog_context.labelColors[logLevel][OKIMPLLOG_FG_INDEX_ANSI] = fgColor;
        okimpllog_context.labelColors[logLevel][OKIMPLLOG_FG_INDEX_ANSI256] = fgColor256;
        okimpllog_context.labelColors[logLevel][OKIMPLLOG_BG_INDEX_ANSI] = bgColor;
        okimpllog_context.labelColors[logLevel][OKIMPLLOG_BG_INDEX_ANSI256] = bgColor256;

        // This is a bit inefficient as it always updates all log levels.
        okimpllog_updateLogLevelColorCache();
    }
}

OKIMPLLOG_DEF void oklog_setMessageColor(OKLOG_LogLevel logLevel, OKLOG_Color fgColor, int fgColor256, OKLOG_Color bgColor, int bgColor256) {
    if (logLevel > OKLOG_LEVEL_ALL && logLevel < OKLOG_LEVEL_SILENT) {
        okimpllog_confirmInit();

        okimpllog_context.messageColors[logLevel][OKIMPLLOG_FG_INDEX_ANSI] = fgColor;
        okimpllog_context.messageColors[logLevel][OKIMPLLOG_FG_INDEX_ANSI256] = fgColor256;
        okimpllog_context.messageColors[logLevel][OKIMPLLOG_BG_INDEX_ANSI] = bgColor;
        okimpllog_context.messageColors[logLevel][OKIMPLLOG_BG_INDEX_ANSI256] = bgColor256;

        // This is a bit inefficient as it always updates all log levels.
        okimpllog_updateLogLevelColorCache();
    }
}

//
// Internal helper functions.
//

OKIMPLLOG_DEF_INTERNAL void okimpllog_confirmInit() {
    if (okimpllog_context.initDone) {
        return;
    }

    // NOTE: that must set initDone before calling own methods to not loop indefinitely.
    okimpllog_context.initDone = 1;

#if _WIN32 || _WIN64
    // Set utf-8 code page on windows console. This should only affect this program.
    SetConsoleOutputCP(65001);
#endif

    // Reset all colors to "none".
    for (int i = 0; i < OKIMPLLOG_NUM_LOG_LEVELS; ++i) {
        // Reset all color mappings to "no coloring" (as 0 would be a black color).
        for (size_t j = 0; j < 4; ++j) {
            okimpllog_context.labelColors[i][j] = -1;
            okimpllog_context.messageColors[i][j] = -1;
        }
    }
    // Set some default message colors.
    okimpllog_context.labelColors[OKLOG_LEVEL_VERBOSE][OKIMPLLOG_FG_INDEX_ANSI] = OKLOG_COLOR_BRIGHT_CYAN;
    okimpllog_context.labelColors[OKLOG_LEVEL_VERBOSE][OKIMPLLOG_FG_INDEX_ANSI256] = 45;
    okimpllog_context.labelColors[OKLOG_LEVEL_DEBUG][OKIMPLLOG_FG_INDEX_ANSI] = OKLOG_COLOR_BRIGHT_CYAN;
    okimpllog_context.labelColors[OKLOG_LEVEL_DEBUG][OKIMPLLOG_FG_INDEX_ANSI256] = 45;
    okimpllog_context.labelColors[OKLOG_LEVEL_INFO][OKIMPLLOG_FG_INDEX_ANSI] = OKLOG_COLOR_BRIGHT_GREEN;
    okimpllog_context.labelColors[OKLOG_LEVEL_INFO][OKIMPLLOG_FG_INDEX_ANSI256] = 40;
    okimpllog_context.labelColors[OKLOG_LEVEL_WARNING][OKIMPLLOG_FG_INDEX_ANSI] = OKLOG_COLOR_BRIGHT_YELLOW;
    okimpllog_context.labelColors[OKLOG_LEVEL_WARNING][OKIMPLLOG_FG_INDEX_ANSI256] = 220;
    okimpllog_context.labelColors[OKLOG_LEVEL_ERROR][OKIMPLLOG_FG_INDEX_ANSI] = OKLOG_COLOR_BRIGHT_RED;
    okimpllog_context.labelColors[OKLOG_LEVEL_ERROR][OKIMPLLOG_FG_INDEX_ANSI256] = 160;
    okimpllog_context.labelColors[OKLOG_LEVEL_FATAL][OKIMPLLOG_FG_INDEX_ANSI] = OKLOG_COLOR_BRIGHT_RED;
    okimpllog_context.labelColors[OKLOG_LEVEL_FATAL][OKIMPLLOG_FG_INDEX_ANSI256] = 255;
    okimpllog_context.labelColors[OKLOG_LEVEL_FATAL][OKIMPLLOG_BG_INDEX_ANSI256] = 160;

    okimpllog_context.messageColors[OKLOG_LEVEL_VERBOSE][OKIMPLLOG_FG_INDEX_ANSI256] = 240;
    okimpllog_context.messageColors[OKLOG_LEVEL_DEBUG][OKIMPLLOG_FG_INDEX_ANSI256] = 240;
    okimpllog_context.messageColors[OKLOG_LEVEL_WARNING][OKIMPLLOG_FG_INDEX_ANSI] = OKLOG_COLOR_BRIGHT_YELLOW;
    okimpllog_context.messageColors[OKLOG_LEVEL_WARNING][OKIMPLLOG_FG_INDEX_ANSI256] = 220;
    okimpllog_context.messageColors[OKLOG_LEVEL_ERROR][OKIMPLLOG_FG_INDEX_ANSI] = OKLOG_COLOR_BRIGHT_RED;
    okimpllog_context.messageColors[OKLOG_LEVEL_ERROR][OKIMPLLOG_FG_INDEX_ANSI256] = 160;
    okimpllog_context.messageColors[OKLOG_LEVEL_FATAL][OKIMPLLOG_FG_INDEX_ANSI] = OKLOG_COLOR_BRIGHT_RED;
    okimpllog_context.messageColors[OKLOG_LEVEL_FATAL][OKIMPLLOG_FG_INDEX_ANSI256] = 255;
    okimpllog_context.messageColors[OKLOG_LEVEL_FATAL][OKIMPLLOG_BG_INDEX_ANSI256] = 160;

    // Set default color mode.
    oklog_setColorMode(OKLOG_MODE_COLOR_AUTO);


    // Set some default formatting options depending on the build type.
#if defined(OKLOG_NO_RELEASE_STRIPPING) || defined(DEBUG)
    okimpllog_context.logLevelFilter = OKLOG_LEVEL_DEBUG;
    oklog_setFormat(OKIMPLLOG_DEFAULT_FORMAT);
#else
    okimpllog_context.logLevelFilter = OKLOG_LEVEL_INFO;
    oklog_setFormat(OKIMPLLOG_DEFAULT_FORMAT_STRIPPED);
#endif
}

OKIMPLLOG_DEF_INTERNAL OKIMPLLOG_LogField* okimpllog_addField() {
    if (okimpllog_context.fieldCount < OKLOG_MAX_FIELD_COUNT) {
        okimpllog_context.fieldCount++;
        OKIMPLLOG_LogField* field = &okimpllog_context.fields[okimpllog_context.fieldCount - 1];

        field->type = OKIMPLLOG_LOG_FIELD_NONE;
        field->formatString = 0;
        field->startPadding = 0;
        *field->padEnd = '\0';
        field->colorType = OKIMPLLOG_TYPE_COLOR_NONE;

        return field;
    }
    return 0;
}

OKIMPLLOG_DEF_INTERNAL const char* okimpllog_getFilename(const char* file, int fullpath) {
    if (fullpath) {
        return file;
    } else {
#if _WIN32 || _WIN64
        char* lastSeparator1 = strrchr(file, '\\');
        char* lastSeparator2 = strrchr(file, '/');
        if (lastSeparator2 == 0 && lastSeparator1 == 0) {
            return file;
        } else if (lastSeparator2 > lastSeparator1) {
            return lastSeparator2 + 1;
        } else {
            return lastSeparator1 + 1;
        }
#else
        char* lastSeparator = strrchr(file, '/');
        if (lastSeparator == 0) {
            return file;
        } else {
            return lastSeparator + 1;
        }
#endif
    }
}

OKIMPLLOG_DEF_INTERNAL int okimpllog_checkLogFilter(OKLOG_LogLevel logLevel, const char* tag) {
    if (okimpllog_context.filterCallback) {
        if (!okimpllog_context.filterCallback(logLevel, tag)) {
            return 0;
        }
    } else if ((logLevel < okimpllog_context.logLevelFilter) || (logLevel >= OKLOG_LEVEL_SILENT)) {
        return 0;
    }
    return 1;
}

//
// These are here because using snprintf is a bit of a nightmare to deal with as the implementation is different on windows.
// (There is most likely a saner way to do this)
//

// Returns the length of the text after the operation or -1 if there was not enough space. Does not write
// anything if there was not enough space.
// space and retu
OKIMPLLOG_DEF_INTERNAL int okimpllog_appendString(char* text, int textLength, size_t bufferSize, const char* value) {
    if (bufferSize == 0) {
        return -1;
    }
    if (textLength < 0) {
        textLength = (int) strlen(text);
    }
    size_t valueLength = strlen(value);

    if ((size_t) textLength + valueLength + 1 <= bufferSize) {
        strcat(text, value);
        return textLength + (int) valueLength;
    }

    return -1;
}

// Returns the length of the newly created string. Does not append if there was not enough space.
OKIMPLLOG_DEF_INTERNAL int okimpllog_appendInt(char* text, int textLength, size_t bufferSize, int value) {
    if (bufferSize == 0) {
        return -1;
    }
    if (textLength < 0) {
        textLength = (int) strlen(text);
    }
    if ((size_t) textLength + 1 >= bufferSize) {
        // No space for new characters. Any number would need at least one character.
        return -1;
    }

    char* numberBeginning = text + textLength;
    char* dst = numberBeginning;
    char* end = text + bufferSize - 1;

    if (value < 0) {
        if (dst + 1 >= end) {
            // No space for the minus character. Reverting back to the original string.
            *numberBeginning = '\0';
            return -1;
        }
        value = -value;
        *dst++ = '-';
    }

    char* firstDigit = dst;
    int tempValue;
    while (1) {
        tempValue = value / 10;
        int digit = value - (tempValue * 10);
        *dst++ = (char) ('0' + digit);
        value = tempValue;

        if (value == 0) {
            break;
        }
        if (dst == end) {
            // No space for the number. Reverting back to the original string.
            *numberBeginning = '\0';
            return -1;
        }
    }

    *dst = '\0';
    int newLength = (int) (dst - text);

    // The number was written in reverse. Fix that.
    dst = firstDigit;
    end = text + newLength - 1;
    char tempChar;
    while (dst < end) {
        tempChar = *dst;
        *dst++ = *end;
        *end-- = tempChar;
    }

    return newLength;
}

//
// Ansi color code resources:
//   https://en.wikipedia.org/wiki/ANSI_escape_code
//   http://www.lihaoyi.com/post/BuildyourownCommandLinewithANSIescapecodes.html
//
OKIMPLLOG_DEF_INTERNAL int okimpllog_generateColorCodeAnsi(char* buffer, size_t bufferSize, int fgColor, int bgColor) {
    if (bufferSize == 0) {
        return 0;
    }
    *buffer = '\0';
    int outputLength = 0;

    int colors[] = {fgColor, bgColor};
    const char* prefixes[] = {"\x1B[3", "\x1B[4"};
    int i;
    for (i = 0; i < 2; i++) {
        int color = colors[i];
        if (color >= 0 && color < 16) {
            outputLength = okimpllog_appendString(buffer, outputLength, bufferSize, prefixes[i]);
            if (outputLength < 0) break;

            if (color < 8) {
                outputLength = okimpllog_appendInt(buffer, outputLength, bufferSize, color);
                if (outputLength < 0) break;

                outputLength = okimpllog_appendString(buffer, outputLength, bufferSize, "m");
                if (outputLength < 0) break;

            } else {
                // Bright color -> move index bac to range 0-7 and add ";1" to the end
                outputLength = okimpllog_appendInt(buffer, outputLength, bufferSize, color - 8);
                if (outputLength < 0) break;

                outputLength = okimpllog_appendString(buffer, outputLength, bufferSize, ";1m");
                if (outputLength < 0) break;
            }
        }
    }

    if (i != 2) {
        // The whole code string did not fit the given buffer. Clear it all as it might be invalid.
        *buffer = '\0';
        return 0;
    }

    return 1;
}

OKIMPLLOG_DEF_INTERNAL int okimpllog_generateColorCodeAnsi256(char* buffer, size_t bufferSize, int fgColor, int bgColor) {
    if (bufferSize == 0) {
        return 0;
    }
    *buffer = '\0';
    int outputLength = 0;

    int colors[] = {fgColor, bgColor};
    const char* prefixes[] = {"\x1B[38;5;", "\x1B[48;5;"};
    int i;
    for (i = 0; i < 2; i++) {
        int color = colors[i];
        if (color >= 0 && color < 256) {
            outputLength = okimpllog_appendString(buffer, outputLength, bufferSize, prefixes[i]);
            if (outputLength < 0) break;

            outputLength = okimpllog_appendInt(buffer, outputLength, bufferSize, color);
            if (outputLength < 0) break;

            outputLength = okimpllog_appendString(buffer, outputLength, bufferSize, "m");
            if (outputLength < 0) break;
        }
    }

    if (i != 2) {
        // The whole code string did not fit the given buffer. Clear it all as it might be invalid.
        *buffer = '\0';
        return 0;
    }

    return 1;
}

OKIMPLLOG_DEF_INTERNAL int okimpllog_generateColorCode(char* buffer, size_t bufferSize, int background, int ansi, int ansi256) {
    if (!buffer || bufferSize <= 0) {
        return 0;
    }
    switch (okimpllog_context.colorMode) {
        case OKLOG_MODE_COLOR_ANSI:
            return okimpllog_generateColorCodeAnsi(buffer, bufferSize, background ? -1 : ansi, background ? ansi : -1);

        case OKLOG_MODE_COLOR_ANSI256:
            return okimpllog_generateColorCodeAnsi256(buffer, bufferSize, background ? -1 : ansi256, background ? ansi256 : -1);

        default:
            *buffer = '\0';
            return 0;
    }
}

OKIMPLLOG_DEF_INTERNAL void okimpllog_updateFieldColorCache() {
    if (okimpllog_context.colorMode == OKLOG_MODE_COLOR_ANSI) {
        for (int i = 0; i < okimpllog_context.fieldCount; ++i) {
            if (okimpllog_context.fields[i].colorType == OKIMPLLOG_TYPE_COLOR_CODE) {
                okimpllog_generateColorCodeAnsi(
                        okimpllog_context.fields[i].colorCode, sizeof(okimpllog_context.fields[0].colorCode),
                        okimpllog_context.fields[i].color[OKIMPLLOG_FG_INDEX_ANSI],
                        okimpllog_context.fields[i].color[OKIMPLLOG_BG_INDEX_ANSI]
                );
            }
        }

    } else if (okimpllog_context.colorMode == OKLOG_MODE_COLOR_ANSI256) {
        for (int i = 0; i < okimpllog_context.fieldCount; ++i) {
            if (okimpllog_context.fields[i].colorType == OKIMPLLOG_TYPE_COLOR_CODE) {
                okimpllog_generateColorCodeAnsi256(
                        okimpllog_context.fields[i].colorCode, sizeof(okimpllog_context.fields[0].colorCode),
                        okimpllog_context.fields[i].color[OKIMPLLOG_FG_INDEX_ANSI256],
                        okimpllog_context.fields[i].color[OKIMPLLOG_BG_INDEX_ANSI256]
                );
            }
        }

    } else {
        for (int i = 0; i < okimpllog_context.fieldCount; ++i) {
            *okimpllog_context.fields[i].colorCode = '\0';
        }
    }
}

OKIMPLLOG_DEF_INTERNAL void okimpllog_updateLogLevelColorCache() {
    if (okimpllog_context.colorMode == OKLOG_MODE_COLOR_ANSI) {
        for (int i = 0; i < OKIMPLLOG_NUM_LOG_LEVELS; ++i) {
            okimpllog_generateColorCodeAnsi(
                    okimpllog_context.labelColorCode[i], sizeof(okimpllog_context.labelColorCode[0]),
                    okimpllog_context.labelColors[i][OKIMPLLOG_FG_INDEX_ANSI],
                    okimpllog_context.labelColors[i][OKIMPLLOG_BG_INDEX_ANSI]);
            okimpllog_generateColorCodeAnsi(
                    okimpllog_context.messageColorCode[i], sizeof(okimpllog_context.messageColorCode[0]),
                    okimpllog_context.messageColors[i][OKIMPLLOG_FG_INDEX_ANSI],
                    okimpllog_context.messageColors[i][OKIMPLLOG_BG_INDEX_ANSI]);
        }

    } else if (okimpllog_context.colorMode == OKLOG_MODE_COLOR_ANSI256) {
        for (int i = 0; i < OKIMPLLOG_NUM_LOG_LEVELS; ++i) {
            okimpllog_generateColorCodeAnsi256(
                    okimpllog_context.labelColorCode[i], sizeof(okimpllog_context.labelColorCode[0]),
                    okimpllog_context.labelColors[i][OKIMPLLOG_FG_INDEX_ANSI256],
                    okimpllog_context.labelColors[i][OKIMPLLOG_BG_INDEX_ANSI256]);
            okimpllog_generateColorCodeAnsi256(
                    okimpllog_context.messageColorCode[i], sizeof(okimpllog_context.messageColorCode[0]),
                    okimpllog_context.messageColors[i][OKIMPLLOG_FG_INDEX_ANSI256],
                    okimpllog_context.messageColors[i][OKIMPLLOG_BG_INDEX_ANSI256]);
        }

    } else {
        for (int i = 0; i < OKIMPLLOG_NUM_LOG_LEVELS; ++i) {
            *okimpllog_context.labelColorCode[i] = '\0';
            *okimpllog_context.messageColorCode[i] = '\0';
        }
    }
}


#endif // ifdef OKLOG_IMPLEMENTATION


#ifdef __cplusplus
}
#endif

#endif // ifndef OKWRAPPER_OKLOG_H_
