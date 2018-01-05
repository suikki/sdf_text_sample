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

#include "fontstash.h"
#include "gl3corefontstash.h"

#include "ok_platform_wrapper/ok_app.h"
#include "ok_platform_wrapper/ok_log.h"
#include "ok_gl_helper/ok_gl_helper.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// Most SDL functionality is behind a simple wrapper, except for the key constants.
#include <SDL_keycode.h>

#define LOG_TAG "sdf_text_app"

uint64_t timeOffsetMicros = 0;
uint64_t timeMicros = 0;

int windowWidth = 0;
int windowHeight = 0;

int showCache = 0;
int dragging = 0;
float offsetX = 60.0f;
float offsetY = 200.0f;
float targetScale = 1.0f;
float scale = 1.0f;

GLuint shaderColored = 0;
GLuint shaderText = 0;
GLuint shaderTextSdf = 0;
GLuint shaderTextSdfEffects = 0;

uint8_t* fontDataDroidSans = NULL;
uint8_t* fontDataDroidSansJapanese = NULL;

FONScontext* fs = NULL;
int fontNormal = FONS_INVALID;
int fontSdf = FONS_INVALID;
int fontSdfEffects = FONS_INVALID;

// Fontstash callback function.
void fontStashError(void* userPointer, int error, int value);


void releaseShaders() {
    glDeleteProgram(shaderColored);
    shaderColored = 0;

    glDeleteProgram(shaderText);
    shaderText = 0;

    glDeleteProgram(shaderTextSdf);
    shaderTextSdf = 0;

    glDeleteProgram(shaderTextSdfEffects);
    shaderTextSdfEffects = 0;
}

void loadShaders() {
    // Make sure to release old shaders first (might be reloading).
    releaseShaders();

    //
    // A basic shader from a string.
    //
    const char* vShaderColored = ""
            "uniform mat4 modelView;\n"
            "uniform mat4 projection;\n"
            "attribute vec4 vertexPosition;\n"
            "attribute vec4 vertexColor;\n"
            "varying vec4 interpolatedColor;\n"
            "void main() {\n"
            "  interpolatedColor = vertexColor;\n"
            "  gl_Position = projection * modelView * vertexPosition;\n"
            "}\n";

    const char* fShaderColored = ""
            "varying vec4 interpolatedColor;\n"
            "void main() {\n"
            "  gl_FragColor = interpolatedColor;\n"
            "  gl_FragColor.rgb *= gl_FragColor.a;\n" // Using premultiplied alpha.
            "}\n";

    shaderColored = okgl_linkProgram(vShaderColored, fShaderColored);

    //
    // Load some shaders from assets files.
    //
    char* vShaderText = okapp_loadTextAsset("shaders/text.v.glsl");
    char* fShaderText = okapp_loadTextAsset("shaders/text.f.glsl");
    char* fShaderTextSdf = okapp_loadTextAsset("shaders/text_sdf.f.glsl");
    char* fShaderTextSdfEffects = okapp_loadTextAsset("shaders/text_sdf_effects.f.glsl");

    shaderText = okgl_linkProgram(vShaderText, fShaderText);
    shaderTextSdf = okgl_linkProgram(vShaderText, fShaderTextSdf);
    shaderTextSdfEffects = okgl_linkProgram(vShaderText, fShaderTextSdfEffects);

    free(vShaderText);
    free(fShaderText);
    free(fShaderTextSdf);
    free(fShaderTextSdfEffects);
}

void releaseFonts() {
    if (fs) {
        glfonsDelete(fs);
        fs = NULL;
    }

    free(fontDataDroidSans);
    fontDataDroidSans = NULL;

    free(fontDataDroidSansJapanese);
    fontDataDroidSansJapanese = NULL;
}

int loadFonts() {
    // Make sure to release old fonts first.
    releaseFonts();

    //
    // Initialize fontstash.
    //
    fs = glfonsCreate(512, 512, FONS_ZERO_TOPLEFT);
    if (fs == NULL) {
        oklog_e(LOG_TAG, "Could not create font stash.\n");
        return 0;
    }

    fonsSetErrorCallback(fs, fontStashError, fs);

    //
    // Load font data.
    //

    const char* droidSansFilename = "fonts/droid/DroidSans.ttf";
    fontDataDroidSans = NULL;
    int fontDataDroidSansSize = okapp_loadBinaryAsset(droidSansFilename, &fontDataDroidSans);
    if (fontDataDroidSansSize <= 0) {
        oklog_e(LOG_TAG, "Error reading font file: '%s'", droidSansFilename);
        return 0;
    }

    const char* droidSansJapaneseFilename = "fonts/droid/DroidSansJapanese.ttf";
    fontDataDroidSansJapanese = NULL;
    int fontDataDroidSansJapaneseSize = okapp_loadBinaryAsset(droidSansJapaneseFilename, &fontDataDroidSansJapanese);
    if (fontDataDroidSansJapaneseSize <= 0) {
        oklog_e(LOG_TAG, "Error reading font file: '%s'", droidSansJapaneseFilename);
        return 0;
    }

    //
    // Create fonts.
    //

    // Note that we tell fontstash to not free the memory after it's done with the font, because we reuse the
    // data for multiple fonts.
    int callFree = 0;

    // Font1: no SDF, not supporting Japanese.
    FONSsdfSettings noSdf = {0};
    noSdf.sdfEnabled = 0;

    fontNormal = fonsAddFontSdfMem(fs, "DroidSans", fontDataDroidSans, fontDataDroidSansSize, callFree, noSdf);
    if (fontNormal == FONS_INVALID) {
        oklog_e(LOG_TAG, "Could not add font.\n");
        return 0;
    }

    // Font2: Basic SDF support, also supporting Japanese.
    // (The very small padding enables basicSDF rendering but not effects like outlines properly).
    FONSsdfSettings basicSdf = {0};
    basicSdf.sdfEnabled = 1;
    basicSdf.onedgeValue = 127;
    basicSdf.padding = 1;
    basicSdf.pixelDistScale = 62.0;

    fontSdf = fonsAddFontSdfMem(fs, "DroidSansSdf", fontDataDroidSans, fontDataDroidSansSize, callFree, basicSdf);
    if (fontSdf == FONS_INVALID) {
        oklog_e(LOG_TAG, "Could not add SDF font.\n");
        return 0;
    } else {
        int fontJPSdf = fonsAddFontSdfMem(fs, "DroidSansSdfJP", fontDataDroidSansJapanese, fontDataDroidSansJapaneseSize, callFree, basicSdf);
        if (fontJPSdf == FONS_INVALID) {
            oklog_e(LOG_TAG, "Could not add japanese SDF font.\n");
            return 0;
        }
        fonsAddFallbackFont(fs, fontSdf, fontJPSdf);
    }

    // Font3: SDF support, also supporting Japanese.
    FONSsdfSettings effectsSdf = {0};
    effectsSdf.sdfEnabled = 1;
    effectsSdf.onedgeValue = 127;
    effectsSdf.padding = 10;
    effectsSdf.pixelDistScale = 8.0;

    fontSdfEffects = fonsAddFontSdfMem(fs, "DroidSansSdfEffects", fontDataDroidSans, fontDataDroidSansSize, callFree, effectsSdf);
    if (fontSdf == FONS_INVALID) {
        oklog_e(LOG_TAG, "Could not add SDF font.\n");
        return 0;
    } else {
        int fontJPSdf = fonsAddFontSdfMem(fs, "DroidSansSdfEffectsJP", fontDataDroidSansJapanese, fontDataDroidSansJapaneseSize, callFree, effectsSdf);
        if (fontJPSdf == FONS_INVALID) {
            oklog_e(LOG_TAG, "Could not add japanese SDF font.\n");
            return 0;
        }
        fonsAddFallbackFont(fs, fontSdfEffects, fontJPSdf);
    }

    return 1;
}


//
// App callbacks.
//

void onStart(int argc, char* argv[]) {
    oklog_i(LOG_TAG, "onStart");

    timeMicros = 0;

    // Print OpenGL driver information.
    okgl_logInfo();

    loadShaders();

    if (!loadFonts()) {
        okapp_queueQuit();
    }
}

void onStop() {
    oklog_i(LOG_TAG, "onStop");
    releaseFonts();
    releaseShaders();
}

void onSizeChanged(int width, int height) {
    oklog_i(LOG_TAG, "onSizeChanged %d %d", width, height);
    windowWidth = width;
    windowHeight = height;
}

void onKeyEvent(OKAPP_KeyEvent keyEvent) {
    if (keyEvent.eventType == OKAPP_KEY_DOWN) {
        if (keyEvent.keyCode == SDLK_ESCAPE) {
            okapp_queueQuit();

        } else if (keyEvent.keyCode == 'r') {
            oklog_i(LOG_TAG, "Reloading shaders.");
            loadShaders();

        } else if (keyEvent.keyCode == 'c') {
            showCache = !showCache;

        } else if (keyEvent.keyCode == 'f') {
            if (okapp_getWindowMode() != OKAPP_WINDOW_MODE_FULLSCREEN_WINDOW) {
                okapp_setWindowMode(OKAPP_WINDOW_MODE_FULLSCREEN_WINDOW);
            } else {
                okapp_setWindowMode(OKAPP_WINDOW_MODE_WINDOWED);
            }
        }
    }
}

void onPointerEvent(OKAPP_PointerEvent pointerEvent) {
    if (pointerEvent.eventType == OKAPP_POINTER_DOWN) {
        dragging = 1;

    } else if (pointerEvent.eventType == OKAPP_POINTER_UP) {
        dragging = 0;

    } else if (pointerEvent.eventType == OKAPP_POINTER_MOVE) {
        if (dragging) {
            if (scale != 0.0f) {
                offsetX += pointerEvent.deltaX / scale;
                offsetY += pointerEvent.deltaY / scale;
            }
        }

    } else if (pointerEvent.eventType == OKAPP_POINTER_SCROLL) {
        targetScale *= 1.0f + (pointerEvent.y * 0.15f);
        if (targetScale > 100.0f) {
            targetScale = 100.0f;
        }
    }
}

void onRender() {
    // Some kind of simple time measurement.
    uint64_t now = okapp_getTimerMicros();
    uint64_t lastFrameTimeMicros = 0;
    if (timeOffsetMicros == 0) {
        timeOffsetMicros = now;
    } else {
        lastFrameTimeMicros = now - timeMicros;
    }
    timeMicros = now;
    // Floats will lose precision after time get bigger, but this is good enough for this demo.
    float deltaT = (float) (lastFrameTimeMicros * 0.000001);
    float timeSeconds = (float) ((timeMicros - timeOffsetMicros) * 0.000001);

    // Smoothing the zoom a bit.
    if (deltaT > 0.0f) {
        float smoothing = powf(0.9f, (float) (deltaT * 60.0f));
        scale = (targetScale * (1.0f - smoothing)) + (scale * smoothing);
    }

    glViewport(0, 0, windowWidth, windowHeight);

    // This creates a coordinate system where one pixel is one unit and where (0, 0) is the top left corner.
    GLfloat projection[16];
    okgl_projection2d(projection, 0.0f, (GLfloat) windowWidth, (GLfloat) windowHeight, 0.0f);

    GLfloat modelView[16];
    okgl_unitMatrix(modelView);

    //
    // Clear the screen.
    //
    glClearColor(0.4f, 0.4f, 0.42f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    //
    // Enable blending.
    //
    glEnable(GL_BLEND);
    // Note: using premultiplied alpha blending.
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);


    // Scale text with time and move with mouse.
    //float scale = (1.01f + cosf(((float) zoomAnimationTime) * 0.7f)) * 2.2f;
    //float scale = 1.0f;
    okgl_matrixSetscale(modelView, scale, scale, 0.0f);
    okgl_matrixSetTranslation(modelView,
                              offsetX + (1.0f - scale) * (windowWidth * 0.5f - offsetX),
                              offsetY + (1.0f - scale) * (windowHeight * 0.5f - offsetY),
                              0.0f);

    float lineHeight = 0.0f;
    float x = 0.0f;
    float y = 0.0f;

    {
        //
        // Draw normal text.
        //
        glUseProgram(shaderText);

        GLint projectionMatrixLoc = glGetUniformLocation(shaderText, "projection");
        glUniformMatrix4fv(projectionMatrixLoc, 1, GL_FALSE, &projection[0]);

        GLint modelViewMatrixLoc = glGetUniformLocation(shaderText, "modelView");
        glUniformMatrix4fv(modelViewMatrixLoc, 1, GL_FALSE, &modelView[0]);

        fonsClearState(fs);
        fonsSetFont(fs, fontNormal);
        fonsSetSize(fs, 65.0f);
        fonsSetAlign(fs, FONS_ALIGN_LEFT | FONS_ALIGN_TOP);
        fonsVertMetrics(fs, NULL, NULL, &lineHeight);

        fonsSetColor(fs, glfonsRGBA(255, 0, 0, 255));
        x = fonsDrawText(fs, x, y, "Lorem ", NULL);

        fonsSetColor(fs, glfonsRGBA(255, 255, 0, 255));
        x = fonsDrawText(fs, x, y, "ipsum ", NULL);

        fonsSetColor(fs, glfonsRGBA(255, 0, 0, 255));
        x = fonsDrawText(fs, x, y, "dolor sit amet (not SDF)", NULL);

        x = 0.0f;
        y += lineHeight;
    }

    {
        //
        // Draw SDF text.
        //
        glUseProgram(shaderTextSdf);

        GLint projectionMatrixLoc = glGetUniformLocation(shaderTextSdf, "projection");
        glUniformMatrix4fv(projectionMatrixLoc, 1, GL_FALSE, &projection[0]);

        GLint modelViewMatrixLoc = glGetUniformLocation(shaderTextSdf, "modelView");
        glUniformMatrix4fv(modelViewMatrixLoc, 1, GL_FALSE, &modelView[0]);

        fonsClearState(fs);
        fonsSetFont(fs, fontSdf);
        fonsSetSize(fs, 65.0f);
        fonsSetAlign(fs, FONS_ALIGN_LEFT | FONS_ALIGN_TOP);
        fonsVertMetrics(fs, NULL, NULL, &lineHeight);

        fonsSetColor(fs, glfonsRGBA(255, 0, 0, 255));
        x = fonsDrawText(fs, x, y, "Lorem ", NULL);

        fonsSetColor(fs, glfonsRGBA(255, 255, 0, 255));
        x = fonsDrawText(fs, x, y, "ipsum ", NULL);

        fonsSetColor(fs, glfonsRGBA(255, 0, 0, 255));
        x = fonsDrawText(fs, x, y, "dolor sit amet (SDF)", NULL);


        char dynamicText[] = {1, '\0', '\0'};
        dynamicText[0] += ((int) (timeSeconds * 10.0)) % 127;
        dynamicText[1] += ((int) (timeSeconds * 15.0)) % 128;
        x = fonsDrawText(fs, x, y, dynamicText, NULL);

        x = 0.0f;
        y += lineHeight;

        fonsSetColor(fs, glfonsRGBA(102, 255, 204, 255));
        x = fonsDrawText(fs, x, y, "Japanese: 点おやをづ例声念ヒレル試石べ位掲質", NULL);

        x = 0.0f;
        y += lineHeight;

        fonsSetColor(fs, glfonsRGBA(12, 24, 25, 255));
        x = fonsDrawText(fs, x, y, "Cyrillic: Лорем ипсум долор сит амет, иус ет", NULL);

        x = 0.0f;
        y += lineHeight;
    }

    {
        //
        // Draw SDF text with some effects.
        //
        glUseProgram(shaderTextSdfEffects);

        GLint projectionMatrixLoc = glGetUniformLocation(shaderTextSdfEffects, "projection");
        glUniformMatrix4fv(projectionMatrixLoc, 1, GL_FALSE, &projection[0]);

        GLint modelViewMatrixLoc = glGetUniformLocation(shaderTextSdfEffects, "modelView");
        glUniformMatrix4fv(modelViewMatrixLoc, 1, GL_FALSE, &modelView[0]);

        GLint timeLoc = glGetUniformLocation(shaderTextSdfEffects, "time");
        glUniform1f(timeLoc, timeSeconds);

        fonsClearState(fs);
        fonsSetFont(fs, fontSdfEffects);
        fonsSetSize(fs, 65.0f);
        fonsSetAlign(fs, FONS_ALIGN_LEFT | FONS_ALIGN_TOP);
        fonsVertMetrics(fs, NULL, NULL, &lineHeight);

        fonsSetColor(fs, glfonsRGBA(102, 255, 204, 255));
        x = fonsDrawText(fs, x, y, "Japanese: 点おやをづ例声念ヒレル試石べ位掲質", NULL);

        x = 0.0f;
        y += lineHeight;

        fonsSetColor(fs, glfonsRGBA(255, 255, 0, 255));
        x = fonsDrawText(fs, x, y, "Drag ", NULL);

        fonsSetColor(fs, glfonsRGBA(255, 0, 0, 255));
        x = fonsDrawText(fs, x, y, "to move", NULL);

        x = 0.0f;
        y += lineHeight;
    }

    // Reset translation and scale.
    okgl_unitMatrix(modelView);

    if (showCache) {
        //
        // Draw the font cache (for debugging).
        //
        glUseProgram(shaderText);

        GLint projectionMatrixLoc = glGetUniformLocation(shaderText, "projection");
        glUniformMatrix4fv(projectionMatrixLoc, 1, GL_FALSE, &projection[0]);

        // Show the cache texture atlas in the right edge of the window.
        int atlasW, atlasH;
        fonsGetAtlasSize(fs, &atlasW, &atlasH);
        okgl_matrixSetTranslation(modelView, (float) windowWidth - atlasW, 0.0f, 0.0f);

        GLint modelViewMatrixLoc = glGetUniformLocation(shaderText, "modelView");
        glUniformMatrix4fv(modelViewMatrixLoc, 1, GL_FALSE, &modelView[0]);

        fonsDrawDebug(fs, 10, 10);

        // Reset translation.
        okgl_unitMatrix(modelView);
    }

    // Draw some instruction texts.
    {
        glUseProgram(shaderText);

        GLint projectionMatrixLoc = glGetUniformLocation(shaderText, "projection");
        glUniformMatrix4fv(projectionMatrixLoc, 1, GL_FALSE, &projection[0]);

        GLint modelViewMatrixLoc = glGetUniformLocation(shaderText, "modelView");
        glUniformMatrix4fv(modelViewMatrixLoc, 1, GL_FALSE, &modelView[0]);

        fonsClearState(fs);
        fonsSetFont(fs, fontNormal);
        fonsSetSize(fs, 20.0f);
        fonsSetAlign(fs, FONS_ALIGN_LEFT | FONS_ALIGN_TOP);
        fonsVertMetrics(fs, NULL, NULL, &lineHeight);
        lineHeight *= 1.2f;

        y = 5.0f;

        // First draw a shadow.
        fonsSetColor(fs, glfonsRGBA(0, 0, 0, 255));
        fonsSetBlur(fs, 3.0f);

        float yStart = y;
        for (int i = 0; i < 2; ++i) {
            x = 5.0f;
            y = yStart;

            fonsDrawText(fs, x, y, "drag to pan, zoom with mouse wheel", NULL);
            y += lineHeight;
            fonsDrawText(fs, x, y, "'c' - show font cache", NULL);
            y += lineHeight;
            fonsDrawText(fs, x, y, "'f' - toggle fullscreen", NULL);
            y += lineHeight;
            fonsDrawText(fs, x, y, "'r' - reload shaders", NULL);
            y += lineHeight;

            // Draw again without blurring
            fonsSetColor(fs, glfonsRGBA(255, 255, 255, 255));
            fonsSetBlur(fs, 0.0f);
        }

        // Show fps.
        x = 5.0f;
        fonsSetColor(fs, glfonsRGBA(57, 57, 57, 255));
        char fps[10];
        snprintf(fps, 10, "%.2f", (1.0f / deltaT));
        x += fonsDrawText(fs, x, y, "fps: ", NULL);
        fonsDrawText(fs, x, y, fps, NULL);
    }

    glDisable(GL_BLEND);
}

OKAPP_AppSetup okapp_setup() {
    OKAPP_AppSetup setup = {0};
    setup.defaultWindowTitle = "sdf_text";
    setup.prefWidth = 1280;
    setup.prefHeight = 800;

    setup.onStart = onStart;
    setup.onStop = onStop;
    setup.onSizeChanged = onSizeChanged;
    setup.onKeyEvent = onKeyEvent;
    setup.onPointerEvent = onPointerEvent;
    setup.onRender = onRender;

    return setup;
}


//
// Fontstash callbacks.
//

void fontStashResetAtlas(FONScontext* stash, int width, int height) {
    fonsResetAtlas(stash, width, height);
    oklog_i(LOG_TAG, "reset atlas to %d x %d\n", width, height);
}

void fontStashEexpandAtlas(FONScontext* stash) {
    int w = 0, h = 0;
    const int maxTexturesize = okgl_getInt(GL_MAX_TEXTURE_SIZE);

    fonsGetAtlasSize(stash, &w, &h);
    if (w < h) {
        w *= 2;
    } else {
        h *= 2;
    }

    if (w > maxTexturesize || h > maxTexturesize) {
        fontStashResetAtlas(stash, maxTexturesize, maxTexturesize);
    } else {
        fonsExpandAtlas(stash, w, h);
        oklog_i(LOG_TAG, "expanded atlas to %d x %d\n", w, h);
    }
}

void fontStashError(void* userPointer, int error, int value) {
    FONScontext* stash = (FONScontext*) userPointer;
    switch (error) {
        case FONS_ATLAS_FULL:
            oklog_i(LOG_TAG, "Font atlas full.\n");
            fontStashEexpandAtlas(stash);
            break;
        case FONS_SCRATCH_FULL:
            oklog_e(LOG_TAG, "Font error: scratch full, tried to allocate %d\n", value);
            break;
        case FONS_STATES_OVERFLOW:
            oklog_e(LOG_TAG, "Font error: states overflow.\n");
            break;
        case FONS_STATES_UNDERFLOW:
            oklog_e(LOG_TAG, "Font error: states underflow.\n");
            break;
        default:
            oklog_e(LOG_TAG, "Font error: unknown.\n");
            break;
    }
}
