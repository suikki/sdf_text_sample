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
//   #define OKAPP_IMPLEMENTATION
// before you include this file in *one* C or C++ file to create the
// implementation.
//
// i.e. it should look like this:
// #define OKAPP_IMPLEMENTATION
// #include "okwrapper/okapp.h"
//
//
// This is a header only wrapper for creating simple cross platform apps.
// The implementation of the library is using SDL2 that needs to be linked
// to the compiled binary.
//
// Note that you need to have the define before the file might be included from
// other included files too, because the include guard prevents including later
// with the implementation. We are not using a separate guard for the
// implementation here to take advantage of any include guard optimizations by
// the compiler (https://gcc.gnu.org/onlinedocs/cppinternals/Guard-Macros.html)
//
//
// # Revision history
//      0.1   (2018-01-16) Not officially released.
//
//
// # Todo list
//   - Keycodes
//

#ifndef OKWRAPPER_OKAPP_H_
#define OKWRAPPER_OKAPP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

#ifdef OKAPP_STATIC
#define OKIMPLAPP_DEF static
#else
#define OKIMPLAPP_DEF extern
#endif

typedef enum OKAPP_KeyEventType {
    OKAPP_KEY_DOWN,
    OKAPP_KEY_UP,
} OKAPP_KeyEventType;

typedef struct OKAPP_KeyEvent {
    OKAPP_KeyEventType eventType;
    int32_t keyCode;
    int32_t scanCode;
} OKAPP_KeyEvent;

typedef enum OKAPP_PointerEventType {
    OKAPP_POINTER_DOWN,
    OKAPP_POINTER_UP,
    OKAPP_POINTER_MOVE,
    OKAPP_POINTER_SCROLL,
} OKAPP_PointerEventType;

typedef struct OKAPP_PointerEvent {
    OKAPP_PointerEventType eventType;
    int sourceId;
    int index;
    float x;
    float y;
    float initialX;
    float initialY;
    float deltaX;
    float deltaY;
} OKAPP_PointerEvent;

typedef enum OKAPP_WindowMode {
    OKAPP_WINDOW_MODE_WINDOWED = 0,
    OKAPP_WINDOW_MODE_FULLSCREEN_WINDOW,
    OKAPP_WINDOW_MODE_FULLSCREEN,
} OKAPP_WindowMode;

typedef struct OKAPP_AppSetup {
    void (* onStart)(int argc, char* argv[]);
    void (* onStop)();
    void (* onPause)();
    void (* onResume)();
    void (* onRender)();
    void (* onSizeChanged)(int, int);
    void (* onKeyEvent)(OKAPP_KeyEvent);
    void (* onPointerEvent)(OKAPP_PointerEvent);

    int prefWidth;
    int prefHeight;
    OKAPP_WindowMode prefWindowMode;
    char* defaultWindowTitle;
} OKAPP_AppSetup;


//
// Note: This function needs to be implemented by the app. The implementation must return a OKAPP_AppSetup struct filled
// with valid information. If a callback is not used it can be NULL.
// 
OKAPP_AppSetup okapp_setup();


//
// The app public API declarations.
//
OKIMPLAPP_DEF void okapp_setWindowMode(OKAPP_WindowMode windowMode);
OKIMPLAPP_DEF OKAPP_WindowMode okapp_getWindowMode();
OKIMPLAPP_DEF void okapp_queueQuit();

OKIMPLAPP_DEF int okapp_getAssetPath(char* outBuffer, size_t bufferSize, const char* fileName);
OKIMPLAPP_DEF int okapp_loadBinaryAsset(const char* fileName, uint8_t** dataOut);
OKIMPLAPP_DEF char* okapp_loadTextAsset(const char* fileName);


OKIMPLAPP_DEF uint32_t okapp_getTimerMillis();
OKIMPLAPP_DEF uint64_t okapp_getTimerMicros();
OKIMPLAPP_DEF int okapp_loadBinaryFile(const char* fileName, uint8_t** dataOut);
OKIMPLAPP_DEF char* okapp_loadTextFile(const char* fileName);


//
// Don't use logging if the oklog logging lib was not found.
//
#if !defined(OKWRAPPER_OKLOG_H_) || defined(OKAPP_NO_LOGGING)
#define OKIMPLAPP_EMPTY_FUNC(...) do {} while (0)
#define okimplapp_logDebug(tag, ...) OKIMPLAPP_EMPTY_FUNC(tag, __VA_ARGS__)
#define okimplapp_logInfo(tag, ...) OKIMPLAPP_EMPTY_FUNC(tag, __VA_ARGS__)
#define okimplapp_logWarning(tag, ...) OKIMPLAPP_EMPTY_FUNC(tag, __VA_ARGS__)
#define okimplapp_logError(tag, ...) OKIMPLAPP_EMPTY_FUNC(tag, __VA_ARGS__)
#else
#define okimplapp_logDebug(tag, ...) oklog_debug(tag, __VA_ARGS__)
#define okimplapp_logInfo(tag, ...) oklog_info(tag, __VA_ARGS__)
#define okimplapp_logWarning(tag, ...) oklog_warning(tag, __VA_ARGS__)
#define okimplapp_logError(tag, ...) oklog_error(tag, __VA_ARGS__)
#endif

//
// ----------------------------------------------------------------------------
// Implementation starts here.
// ----------------------------------------------------------------------------
//

#ifdef OKAPP_IMPLEMENTATION

// TODO: remove this dependency but make sure not using anything from it first
#include "okgl.h"

#include <SDL.h>

#include <stdio.h>

#define OKIMPLAPP_DEF_INTERNAL static
#define OKIMPLAPP_DATA_INTERNAL static

#define OKIMPLAPP_LOG_TAG "okapp"


typedef struct OKAPP_Context {
    SDL_Window* sdlWindow;
    SDL_GLContext sdlGLContext;
    char* sdlBasePath;
    char* sdlSettingsPath;
    int done;
} OKAPP_Context;

OKIMPLAPP_DATA_INTERNAL OKAPP_Context okapp_context = {0};

OKIMPLAPP_DATA_INTERNAL OKAPP_AppSetup okapp_appSetup = {0};

OKIMPLAPP_DEF void okapp_setWindowMode(OKAPP_WindowMode windowMode) {
    uint32_t windowFlags = 0;

    switch (windowMode) {
        case OKAPP_WINDOW_MODE_WINDOWED:
            windowFlags = 0;
            break;

        case OKAPP_WINDOW_MODE_FULLSCREEN_WINDOW:
            windowFlags = SDL_WINDOW_FULLSCREEN_DESKTOP;
            break;

        case OKAPP_WINDOW_MODE_FULLSCREEN:
            windowFlags = SDL_WINDOW_FULLSCREEN;
            break;
    }

    SDL_SetWindowFullscreen(okapp_context.sdlWindow, windowFlags);
}

OKIMPLAPP_DEF OKAPP_WindowMode okapp_getWindowMode() {
    uint32_t windowFlags = SDL_GetWindowFlags(okapp_context.sdlWindow);

    if ((windowFlags & SDL_WINDOW_FULLSCREEN_DESKTOP) != 0) {
        return OKAPP_WINDOW_MODE_FULLSCREEN_WINDOW;

    } else if ((windowFlags & SDL_WINDOW_FULLSCREEN) != 0) {
        return OKAPP_WINDOW_MODE_FULLSCREEN;
    }

    return OKAPP_WINDOW_MODE_WINDOWED;
}

OKIMPLAPP_DEF void okapp_queueQuit() {
    okapp_context.done = 1;

#if defined(EMSCRIPTEN)
    emscripten_cancel_main_loop();
#endif
}

OKIMPLAPP_DEF uint32_t okapp_getTimerMillis() {
    return SDL_GetTicks();
}

OKIMPLAPP_DEF uint64_t okapp_getTimerMicros() {
    return SDL_GetPerformanceCounter() * 1000000LL / SDL_GetPerformanceFrequency();
}

// Replace all instances of character in a string.
OKIMPLAPP_DEF_INTERNAL void okimplapp_replaceChars(char* string, char from, char to) {
    while (*string != '\0') {
        if (*string == from) {
            *string = to;
        }
        string++;
    }
}

OKIMPLAPP_DEF int okapp_getAssetPath(char* outBuffer, size_t bufferSize, const char* fileName) {

    if (fileName == NULL) {
        return -1;
    }

// TODO: remove dependency
#ifndef OKPLATFORM_ANDROID
#error okplatform.h needed
#endif

#if OKPLATFORM_ANDROID
    // Special handling for Android where the assets are included in the apk.
    return snprintf(outBuffer, bufferSize, "%s", fileName);

#elif OKPLATFORM_EMSCRIPTEN
    // Special handling for emscripten for embedded assets.
    return snprintf(outBuffer, bufferSize, "assets/%s", fileName);

#else
    if (!okapp_context.sdlBasePath) {
        // Platform path not initialized.
        return -1;
    }

    return snprintf(outBuffer, bufferSize, "%s../../assets/%s", okapp_context.sdlBasePath, fileName);
#endif
}

OKIMPLAPP_DEF_INTERNAL char* okimplapp_mallocAssetPath(const char* fileName) {
    int pathLength = okapp_getAssetPath(NULL, 0, fileName);
    if (pathLength > 0) {
        size_t bufferSizeBytes = (pathLength + 1) * sizeof(char);
        char* buffer = (char*) malloc(bufferSizeBytes);
        pathLength = okapp_getAssetPath(buffer, bufferSizeBytes, fileName);
        if (pathLength > 0) {
            return buffer;
        } else {
            free(buffer);
        }
    }
    return 0;
}

OKIMPLAPP_DEF int okapp_loadBinaryAsset(const char* fileName, uint8_t** dataOut) {
    char* path = okimplapp_mallocAssetPath(fileName);
    int result = okapp_loadBinaryFile(path, dataOut);
    free(path);
    return result;
}

OKIMPLAPP_DEF int okapp_loadBinaryFile(const char* fileName, uint8_t** dataOut) {
    *dataOut = NULL;

    SDL_RWops* rw = SDL_RWFromFile(fileName, "rb");
    if (!rw) {
        return -1;
    }

    // Limiting to int size for now. No massive files.
    int fileSize = (int) SDL_RWsize(rw);
    *dataOut = (uint8_t*) malloc((size_t) fileSize);

    int totalRead = 0;
    uint8_t* buf = *dataOut;
    while (totalRead < fileSize) {
        Sint64 read = SDL_RWread(rw, buf, 1, (size_t) (fileSize - totalRead));
        if (read == 0) {
            break;
        }
        totalRead += (int) read;
        buf += read;
    }
    SDL_RWclose(rw);
    if (totalRead != fileSize) {
        free(dataOut);
        *dataOut = NULL;
        return -1;
    }

    return totalRead;
}

OKIMPLAPP_DEF char* okapp_loadTextAsset(const char* fileName) {
    char* path = okimplapp_mallocAssetPath(fileName);
    char* result = okapp_loadTextFile(path);
    free(path);
    return result;
}

OKIMPLAPP_DEF char* okapp_loadTextFile(const char* fileName) {

    SDL_RWops* rw = SDL_RWFromFile(fileName, "r");
    if (!rw) {
        return NULL;
    }

    // Limiting to int size for now. No massive files.
    int fileSize = (int) SDL_RWsize(rw);
    char* dataOut = (char*) malloc((size_t) (fileSize + 1));

    int totalRead = 0;
    char* buf = dataOut;
    while (totalRead < fileSize) {
        Sint64 read = SDL_RWread(rw, buf, 1, (size_t) (fileSize - totalRead));
        if (read == 0) {
            break;
        }
        totalRead += (int) read;
        buf += read;
    }
    SDL_RWclose(rw);
    if (totalRead != fileSize) {
        free(dataOut);
        return NULL;
    }

    dataOut[totalRead] = '\0';
    return dataOut;
}

OKIMPLAPP_DEF_INTERNAL int okimplapp_init() {

    memset(&okapp_context, 0, sizeof(okapp_context));
    okapp_appSetup = okapp_setup();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        okimplapp_logError(OKIMPLAPP_LOG_TAG, "SDL init failed: %s", SDL_GetError());
        SDL_ClearError();
        return 0;
    }


    okapp_context.sdlBasePath = SDL_GetBasePath();
    if (okapp_context.sdlBasePath) {
        // TODO: checkSDLError(__LINE__);
        okimplapp_replaceChars(okapp_context.sdlBasePath, '\\', '/');

        int pathLength = okapp_getAssetPath(NULL, 0, "");
        if (pathLength > 0) {
            size_t bufferSizeBytes = (pathLength + 1) * sizeof(char);
            char* buffer = (char*) malloc(bufferSizeBytes);
            pathLength = okapp_getAssetPath(buffer, bufferSizeBytes, "");
            if (pathLength > 0) {
                okimplapp_logDebug(OKIMPLAPP_LOG_TAG, "Asset path: '%s'", buffer);
            }
            free(buffer);
        }
    }

    // All values are "at least".
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);

#if 0
    // TODO: make requesting alpha channel configurable.
    // Require an alpha channel for testing:
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
#endif

#if 0
    // TODO: make requesting stencil buffer configurable.
    // Seems that a stencil buffer is allocated even if not requested on some platforms.
    // (e.g. iOS uses a combined 24+8 depth+stencil buffer)
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
#endif

#if 0
    // Enable multisampling.
    // TODO: make requesting multisampling configurable.
    // TODO: check if window creation fails and try again without multisampling.
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
#else
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
#endif

    // Request double buffering.
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);


// TODO: pass the wanted opengl profile so that this does not need okgl.hl
#ifndef OKGL_OPENGL_ES
#error okgl.h needed
#endif

#if OKGL_OPENGL_ES
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, OKGL_OPENGL_ES_MAJOR_VERSION);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#endif

    // TODO: make flags configurable.
    Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;

    okapp_context.sdlWindow = SDL_CreateWindow(
            okapp_appSetup.defaultWindowTitle,
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            okapp_appSetup.prefWidth,
            okapp_appSetup.prefHeight,
            flags);
    if (!okapp_context.sdlWindow) {
        okimplapp_logError(OKIMPLAPP_LOG_TAG, "Creating window failed: %s", SDL_GetError());
        SDL_ClearError();
        return 0;
    }

    okapp_context.sdlGLContext = SDL_GL_CreateContext(
            okapp_context.sdlWindow);
    if (!okapp_context.sdlGLContext) {
        okimplapp_logError(OKIMPLAPP_LOG_TAG, "Creating GL context failed: '%s'.", SDL_GetError());
        SDL_ClearError();
        return 0;
    }

    // Enable vsync.
    SDL_GL_SetSwapInterval(1);

    return 1;
}

OKIMPLAPP_DEF_INTERNAL void okimplapp_release() {
    if (okapp_appSetup.onStop) {
        okapp_appSetup.onStop();
    }

    if (okapp_context.sdlGLContext) {
        SDL_GL_DeleteContext(okapp_context.sdlGLContext);
        okapp_context.sdlGLContext = NULL;
    }

    if (okapp_context.sdlWindow) {
        SDL_DestroyWindow(okapp_context.sdlWindow);
        okapp_context.sdlWindow = NULL;
    }

    if (okapp_context.sdlBasePath) {
        SDL_free(okapp_context.sdlBasePath);
    }

    okapp_context.done = 0;

    SDL_Quit();
}

OKIMPLAPP_DEF_INTERNAL void okimplapp_render() {
    if (okapp_context.sdlWindow && okapp_context.sdlGLContext) {
        SDL_GL_MakeCurrent(okapp_context.sdlWindow,
                           okapp_context.sdlGLContext);

        if (okapp_appSetup.onRender) {
            okapp_appSetup.onRender();
        } else {
            // TODO: No render function. What to do?
        }

        SDL_GL_SwapWindow(okapp_context.sdlWindow);
    }
}

OKIMPLAPP_DEF_INTERNAL void okimplapp_sizeChanged() {
    if (okapp_context.sdlWindow && okapp_appSetup.onSizeChanged) {
        int width;
        int height;

        SDL_GL_GetDrawableSize(okapp_context.sdlWindow, &width, &height);
        okapp_appSetup.onSizeChanged(width, height);
    }
}


OKIMPLAPP_DEF_INTERNAL void okimplapp_mainLoop() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                okimplapp_logDebug(OKIMPLAPP_LOG_TAG, "Event: SDL_QUIT");
                okapp_queueQuit();
                break;

            case SDL_KEYDOWN:
                if (okapp_appSetup.onKeyEvent) {
                    OKAPP_KeyEvent appKeyEvent;
                    appKeyEvent.eventType = OKAPP_KEY_DOWN;
                    appKeyEvent.keyCode = event.key.keysym.sym;
                    appKeyEvent.scanCode = event.key.keysym.scancode;
                    okapp_appSetup.onKeyEvent(appKeyEvent);
                }
                break;

            case SDL_KEYUP:
                if (okapp_appSetup.onKeyEvent) {
                    OKAPP_KeyEvent appKeyEvent;
                    appKeyEvent.eventType = OKAPP_KEY_UP;
                    appKeyEvent.keyCode = event.key.keysym.sym;
                    appKeyEvent.scanCode = event.key.keysym.scancode;
                    okapp_appSetup.onKeyEvent(appKeyEvent);
                }

                if (event.key.keysym.sym == SDLK_AC_BACK) {
                    okimplapp_logDebug(OKIMPLAPP_LOG_TAG, "Event: Back key pressed.");
                    // Just quitting in this example. Could move in game UI too.
                    okapp_queueQuit();
                }
                break;

            case SDL_FINGERDOWN:
            case SDL_FINGERUP: {
                if (okapp_appSetup.onPointerEvent) {
                    int windowWidth;
                    int windowHeight;
                    SDL_GL_GetDrawableSize(okapp_context.sdlWindow, &windowWidth, &windowHeight);

                    OKAPP_PointerEvent pointerEvent = {.eventType=(event.type == SDL_FINGERDOWN) ? OKAPP_POINTER_DOWN : OKAPP_POINTER_UP};
                    pointerEvent.sourceId = (int) event.tfinger.touchId;
                    pointerEvent.index = (int) event.tfinger.fingerId;
                    pointerEvent.x = event.tfinger.x * windowWidth;
                    pointerEvent.y = event.tfinger.y * windowHeight;
                    pointerEvent.deltaX = event.tfinger.dx * windowWidth;
                    pointerEvent.deltaY = event.tfinger.dy * windowHeight;
                    okapp_appSetup.onPointerEvent(pointerEvent);
                }
                break;
            }

            case SDL_FINGERMOTION: {
                if (okapp_appSetup.onPointerEvent) {
                    int windowWidth;
                    int windowHeight;
                    SDL_GL_GetDrawableSize(okapp_context.sdlWindow, &windowWidth, &windowHeight);

                    OKAPP_PointerEvent pointerEvent = {.eventType=OKAPP_POINTER_MOVE};
                    pointerEvent.sourceId = (int) event.tfinger.touchId;
                    pointerEvent.index = (int) event.tfinger.fingerId;
                    pointerEvent.x = event.tfinger.x * windowWidth;
                    pointerEvent.y = event.tfinger.y * windowHeight;
                    pointerEvent.deltaX = event.tfinger.dx * windowWidth;
                    pointerEvent.deltaY = event.tfinger.dy * windowHeight;
                    okapp_appSetup.onPointerEvent(pointerEvent);
                }
                break;
            }

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP: {
                // Ignore emulated touch event (already handled).
                if (event.button.which == SDL_TOUCH_MOUSEID) {
                    break;
                }

                if (okapp_appSetup.onPointerEvent) {
                    OKAPP_PointerEvent pointerEvent = {.eventType = (event.type == SDL_MOUSEBUTTONDOWN) ? OKAPP_POINTER_DOWN : OKAPP_POINTER_UP};
                    pointerEvent.sourceId = -1;
                    pointerEvent.index = event.button.button;
                    pointerEvent.x = (float) event.button.x;
                    pointerEvent.y = (float) event.button.y;
                    okapp_appSetup.onPointerEvent(pointerEvent);
                }
                break;
            }

            case SDL_MOUSEMOTION: {
                // Ignore emulated touch event (already handled).
                if (event.motion.which == SDL_TOUCH_MOUSEID) {
                    break;
                }

                if (okapp_appSetup.onPointerEvent) {
                    OKAPP_PointerEvent pointerEvent = {.eventType = OKAPP_POINTER_MOVE};
                    pointerEvent.sourceId = -1;
                    pointerEvent.index = -1;
                    pointerEvent.x = (float) event.motion.x;
                    pointerEvent.y = (float) event.motion.y;
                    pointerEvent.deltaX = (float) event.motion.xrel;
                    pointerEvent.deltaY = (float) event.motion.yrel;
                    okapp_appSetup.onPointerEvent(pointerEvent);
                }
                break;
            }

            case SDL_MOUSEWHEEL: {
                if (okapp_appSetup.onPointerEvent) {
                    OKAPP_PointerEvent pointerEvent = {.eventType=OKAPP_POINTER_SCROLL};
                    pointerEvent.sourceId = -1;
                    pointerEvent.index = -1;
                    pointerEvent.x = (float) event.wheel.x;
                    pointerEvent.y = (float) event.wheel.y;
                    if (pointerEvent.x > 1.0f) {
                        pointerEvent.x = 1.0f;
                    }
                    if (pointerEvent.x < -1.0f) {
                        pointerEvent.x = -1.0f;
                    }
                    if (pointerEvent.y > 1.0f) {
                        pointerEvent.y = 1.0f;
                    }
                    if (pointerEvent.y < -1.0f) {
                        pointerEvent.y = -1.0f;
                    }
                    pointerEvent.deltaX = pointerEvent.x;
                    pointerEvent.deltaY = pointerEvent.y;
                    okapp_appSetup.onPointerEvent(pointerEvent);
                }
                break;
            }

            case SDL_WINDOWEVENT: {
                Uint32 windowID = SDL_GetWindowID(okapp_context.sdlWindow);
                if (event.window.windowID == windowID) {
                    switch (event.window.event) {
                        case SDL_WINDOWEVENT_SIZE_CHANGED:
                            okimplapp_sizeChanged();
                            break;

                        default:
                            break;
                    }
                }
            }

            default:
                break;
        }
    }

    okimplapp_render();
    SDL_Delay(1);
}

OKIMPLAPP_DEF_INTERNAL int okimplapp_main(int argc, char* argv[]) {

    if (!okimplapp_init()) {
        okimplapp_logError(OKIMPLAPP_LOG_TAG, "Initialization failed.");
        // TODO: maybe don't give the exit callback?
        okimplapp_release();
        return 1;
    }

    if (okapp_appSetup.onStart) {
        okapp_appSetup.onStart(argc, argv);
    }

    okimplapp_sizeChanged();

#if defined(EMSCRIPTEN)
    emscripten_set_main_loop(okimplapp_mainLoop, 0, 0);
    return 0;
#else
    while (!okapp_context.done) {
        okimplapp_mainLoop();
    }

    okimplapp_release();
    okimplapp_logDebug(OKIMPLAPP_LOG_TAG, "All done.");
    return 0;
#endif
}

// The actual main is separate to allow not using it.
#ifndef OKAPP_NO_MAIN
int main(int argc, char* argv[]) {
    return okimplapp_main(argc, argv);
}
#endif

#endif // ifdef OKAPP_IMPLEMENTATION

#ifdef __cplusplus
}
#endif

#endif // ifndef OKWRAPPER_OKAPP_H_


