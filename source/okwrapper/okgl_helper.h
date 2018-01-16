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
//   #define OKGL_HELPER_IMPLEMENTATION
// before you include this file in *one* C or C++ file to create the
// implementation.
//
// i.e. it should look like this:
// #define OKGL_HELPER_IMPLEMENTATION
// #include "okwrapper/okgl_helper.h"
//
//
// This is a header only library containing some very basic OpenGL helper
// functions. E.g. for creating shaders.
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

#ifndef OKWRAPPER_OKGL_HELPER_H_
#define OKWRAPPER_OKGL_HELPER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "okgl.h"

#ifdef OKGL_STATIC
#define OKIMPLGL_DEF static
#else
#define OKIMPLGL_DEF extern
#endif

#ifdef DEBUG
#define OKGL_CHECK_ERROR(...) okgl_checkErrorDebug(__FILE__, __LINE__)
#define OKGL_CHECK_ERROR_ALWAYS(...) okgl_checkErrorDebug(__FILE__, __LINE__)
#else
#define OKGL_CHECK_ERROR(...)
#define OKGL_CHECK_ERROR_ALWAYS(...) okgl_checkError()
#endif

OKIMPLGL_DEF int okgl_init();

OKIMPLGL_DEF unsigned int okgl_checkError();
OKIMPLGL_DEF unsigned int okgl_checkErrorDebug(const char* file, int line);
OKIMPLGL_DEF const char* okgl_getErrorString(unsigned int error);

OKIMPLGL_DEF GLint okgl_getInt(GLenum pname);
OKIMPLGL_DEF const char* okgl_getVersion(GLint* glMajorVersion, GLint* glMinorVersion);
OKIMPLGL_DEF void okgl_logInfo();

OKIMPLGL_DEF void okgl_matrixSetTranslation(GLfloat* matrix, GLfloat x, GLfloat y, GLfloat z);
OKIMPLGL_DEF void okgl_matrixSetscale(GLfloat* matrix, GLfloat x, GLfloat y, GLfloat z);
OKIMPLGL_DEF void okgl_unitMatrix(GLfloat* matrix);

OKIMPLGL_DEF void okgl_projection2d(GLfloat* projectionMatrix, GLfloat left, GLfloat right, GLfloat bottom, GLfloat top);

OKIMPLGL_DEF GLuint okgl_createShader(const char* shaderString, GLenum shaderType);
OKIMPLGL_DEF GLuint okgl_linkProgram(const char* vertexShaderString, const char* fragmentShaderString);


//
// Don't use logging if the oklog logging lib was not found.
//
#if !defined(OKWRAPPER_OKLOG_H_) || defined(OKGL_NO_LOGGING)
#define OKIMPLGL_EMPTY_FUNC(...) do {} while (0)
#define okimplgl__logDebug(tag, ...) OKIMPLGL_EMPTY_FUNC(tag, __VA_ARGS__)
#define okimplgl__logInfo(tag, ...) OKIMPLGL_EMPTY_FUNC(tag, __VA_ARGS__)
#define okimplgl__logWarning(tag, ...) OKIMPLGL_EMPTY_FUNC(tag, __VA_ARGS__)
#define okimplgl__logError(tag, ...) OKIMPLGL_EMPTY_FUNC(tag, __VA_ARGS__)
#else
#define okimplgl__logDebug(tag, ...) oklog_debug(tag, __VA_ARGS__)
#define okimplgl__logInfo(tag, ...) oklog_info(tag, __VA_ARGS__)
#define okimplgl__logWarning(tag, ...) oklog_warning(tag, __VA_ARGS__)
#define okimplgl__logError(tag, ...) oklog_error(tag, __VA_ARGS__)
#endif


//
// ----------------------------------------------------------------------------
// Implementation starts here.
// ----------------------------------------------------------------------------
//

#ifdef OKGL_HELPER_IMPLEMENTATION

#include <stdlib.h>

#define OKIMPLGL_DEF_INTERNAL static
#define OKIMPLGL_UNUSED(expr) do { (void)(expr); } while (0)

#define OKIMPLGL_LOG_TAG "okgl_helper"

// TODO: check if init is called properly in each function that needs it
// or call it automatically maybe
// or possibly just in debug mode
OKIMPLGL_DEF int okgl_init() {
#if !OKGL_OPENGL_ES
    // Load OpenGL extensions using GLEW.
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        okimplgl__logError(OKIMPLGL_LOG_TAG, "Error loading glew: %s", glewGetErrorString(err));
        return 0;
    }
    okimplgl__logDebug(OKIMPLGL_LOG_TAG, "GLEW %s loaded.", glewGetString(GLEW_VERSION));

    // GLEW sometimes generates GL errors. Ignore them.
    glGetError();

    // Check that opengl 2.1 is supported.
    if (!GLEW_VERSION_2_1) {
        okimplgl__logError(OKIMPLGL_LOG_TAG, "At least Opengl 2.1 is required to run this program.");
        return 0;
    }
#endif
    return 1;
}

OKIMPLGL_DEF unsigned int okgl_checkError() {
    GLenum error = glGetError();
    if (error) {
        okimplgl__logError(OKIMPLGL_LOG_TAG, "glGetError(%d)\n", error);
    }

    return error;
}

OKIMPLGL_DEF unsigned int okgl_checkErrorDebug(const char* file, int line) {
    OKIMPLGL_UNUSED(file); // when logging is disabled.
    OKIMPLGL_UNUSED(line); // when logging is disabled.

    GLenum error = glGetError();
    if (error) {
        okimplgl__logError(OKIMPLGL_LOG_TAG, "glGetError(%d): %s @ %s line %d\n", error, okgl_getErrorString(error), file, line);
    }

    return error;
}

OKIMPLGL_DEF const char* okgl_getErrorString(unsigned int error) {
    switch (error) {
        case GL_NO_ERROR:
            return "GL_NO_ERROR";

        case GL_INVALID_ENUM:
            return "GL_INVALID_ENUM";

        case GL_INVALID_VALUE:
            return "GL_INVALID_VALUE";

        case GL_INVALID_OPERATION:
            return "GL_INVALID_OPERATION";

        case GL_OUT_OF_MEMORY:
            return "GL_OUT_OF_MEMORY";

        default:
            break;
    }

    return "<unknown>";
}

OKIMPLGL_DEF GLint okgl_getInt(GLenum pname) {
    GLint value;
    glGetIntegerv(pname, &value);
    return value;
}

OKIMPLGL_DEF const char* okgl_getVersion(GLint* glMajorVersion, GLint* glMinorVersion) {

    const char* version = (const char*) glGetString(GL_VERSION);

#if OKGL_OPENGL_ES && (!defined(GL_MAJOR_VERSION) || !defined(GL_MINOR_VERSION))
    // NOTE: this is for old opengl ES versions that dont have GL_MAJOR_VERSION and GL_MINOR_VERSION
    // Assuming it's OpenGL ES2 then (not supporting ES1).
    *glMajorVersion = 2;
    *glMinorVersion = 0;
#else
    glGetIntegerv(GL_MAJOR_VERSION, glMajorVersion);
    glGetIntegerv(GL_MINOR_VERSION, glMinorVersion);
#endif
    OKGL_CHECK_ERROR();

    return version;
}

OKIMPLGL_DEF void okgl_logInfo() {
    GLint glMajorVersion;
    GLint glMinorVersion;
    const char* version;

    version = okgl_getVersion(&glMajorVersion, &glMinorVersion);
    OKIMPLGL_UNUSED(version); // Silence a warning when no logging.

#if OKGL_OPENGL_ES
    okimplgl__logInfo(OKIMPLGL_LOG_TAG, "OpenGL ES %d.%d:", glMajorVersion, glMinorVersion);
#else
    okimplgl__logInfo(OKIMPLGL_LOG_TAG, "OpenGL %d.%d:", glMajorVersion, glMinorVersion);
#endif

    okimplgl__logInfo(OKIMPLGL_LOG_TAG, "  version: %s", version);
    okimplgl__logInfo(OKIMPLGL_LOG_TAG, "  renderer: %s", glGetString(GL_RENDERER));
    okimplgl__logInfo(OKIMPLGL_LOG_TAG, "  vendor: %s", glGetString(GL_VENDOR));
    okimplgl__logInfo(OKIMPLGL_LOG_TAG, "  glsl: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    okimplgl__logInfo(OKIMPLGL_LOG_TAG, "  multisampling: buffers=%d samples=%d", okgl_getInt(GL_SAMPLE_BUFFERS), okgl_getInt(GL_SAMPLES));
    okimplgl__logInfo(OKIMPLGL_LOG_TAG, "  texture image units: %d", okgl_getInt(GL_MAX_TEXTURE_IMAGE_UNITS));
    okimplgl__logInfo(OKIMPLGL_LOG_TAG, "  max texture size: %d", okgl_getInt(GL_MAX_TEXTURE_SIZE));
    okimplgl__logInfo(OKIMPLGL_LOG_TAG, "  max vertex attribs: %d", okgl_getInt(GL_MAX_VERTEX_ATTRIBS));

    if (glMajorVersion < 4) {
        okimplgl__logInfo(OKIMPLGL_LOG_TAG, "  framebuffer: RGB %d%d%d alpha: %d depth: %d stencil: %d",
                          okgl_getInt(GL_RED_BITS), okgl_getInt(GL_GREEN_BITS), okgl_getInt(GL_BLUE_BITS),
                          okgl_getInt(GL_ALPHA_BITS), okgl_getInt(GL_DEPTH_BITS), okgl_getInt(GL_STENCIL_BITS));
    }

    OKGL_CHECK_ERROR();

    {
        GLint supportedTextureFormatCount = okgl_getInt(GL_NUM_COMPRESSED_TEXTURE_FORMATS);
        GLint* supportedFormats = malloc(sizeof(GLint) * supportedTextureFormatCount);

        glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS, supportedFormats);
        okimplgl__logInfo(OKIMPLGL_LOG_TAG, "  some supported texture compression formats:\n");
        for (int i = 0; i < supportedTextureFormatCount; ++i) {
            switch (supportedFormats[i]) {
                case 0x8D64: // GL_ETC1_RGB8_OES
                    okimplgl__logInfo(OKIMPLGL_LOG_TAG, "    ETC1");
                    break;

                case 0x9278: // GL_COMPRESSED_RGBA8_ETC2_EAC
                    okimplgl__logInfo(OKIMPLGL_LOG_TAG, "    ETC2");
                    break;

                case 0x93B0: // GL_COMPRESSED_RGBA_ASTC_4x4_KHR
                    okimplgl__logInfo(OKIMPLGL_LOG_TAG, "    ASTC");
                    break;

                case 0x8C00: // GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG
                    okimplgl__logInfo(OKIMPLGL_LOG_TAG, "    PVRTC1");
                    break;

                case 0x9137: // GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG
                    okimplgl__logInfo(OKIMPLGL_LOG_TAG, "    PVRTC2");
                    break;

                default:
                    break;
            }

            //logc_i(OKIMPLGL_LOG_TAG, "  0x%X\n", supportedFormats[i]);
        }

        free(supportedFormats);
        OKGL_CHECK_ERROR();
    }
}

OKIMPLGL_DEF void okgl_unitMatrix(GLfloat* matrix) {
    matrix[0] = 1.0f;
    matrix[1] = 0.0f;
    matrix[2] = 0.0f;
    matrix[3] = 0.0f;

    matrix[4] = 0.0f;
    matrix[5] = 1.0f;
    matrix[6] = 0.0f;
    matrix[7] = 0.0f;

    matrix[8] = 0.0f;
    matrix[9] = 0.0f;
    matrix[10] = 1.0f;
    matrix[11] = 0.0f;

    matrix[12] = 0.0f;
    matrix[13] = 0.0f;
    matrix[14] = 0.0f;
    matrix[15] = 1.0f;
}

OKIMPLGL_DEF void okgl_matrixSetTranslation(GLfloat* matrix, GLfloat x, GLfloat y, GLfloat z) {
    matrix[12] = x;
    matrix[13] = y;
    matrix[14] = z;
}
OKIMPLGL_DEF void okgl_matrixSetscale(GLfloat* matrix, GLfloat x, GLfloat y, GLfloat z) {
    matrix[0] = x;
    matrix[5] = y;
    matrix[10] = z;
}

OKIMPLGL_DEF void okgl_projection2d(GLfloat* projectionMatrix, GLfloat left, GLfloat right, GLfloat bottom, GLfloat top) {
    GLfloat zRange = (left - right) * 2.0f;
    GLfloat zFar = zRange;
    GLfloat zNear = -zRange;

    projectionMatrix[0] = 2.0f / (right - left);
    projectionMatrix[1] = 0.0f;
    projectionMatrix[2] = 0.0f;
    projectionMatrix[3] = 0.0f;

    projectionMatrix[4] = 0.0f;
    projectionMatrix[5] = 2.0f / (top - bottom);
    projectionMatrix[6] = 0.0f;
    projectionMatrix[7] = 0.0f;

    projectionMatrix[8] = 0.0f;
    projectionMatrix[9] = 0.0f;
    projectionMatrix[10] = -2.0f / (zFar - zNear);
    projectionMatrix[11] = 0.0f;

    projectionMatrix[12] = -(right + left) / (right - left);
    projectionMatrix[13] = -(top + bottom) / (top - bottom);
    projectionMatrix[14] = (zFar + zNear) / (zFar - zNear);
    projectionMatrix[15] = 1.0f;
}

//
// With some minor changes OpenGL ES 2 shaders can be converted to ES 3 / gl3 core shaders.
// This way the same shader can be used on multiple different platforms.
//
// Needs to be freed by the called.
//
OKIMPLGL_DEF char* okgl_convertVertexShader(const char* shaderString,
                                            int outputMajorVersion,
                                            int outputMinorVersion) {
    OKIMPLGL_UNUSED(outputMinorVersion);

    char* preamble = "";
    char* output;

    if (!shaderString) {
        return NULL;
    }

    // TODO: better mappings for different versions.

#if OKGL_OPENGL_ES
    if (outputMajorVersion == 2) {
        preamble =
                "#version 100\n" \
                "precision mediump float;\n" \
                "#define OPENGL_ES\n" \
                "#define OPENGL_ES2\n";

    } else if (outputMajorVersion >= 3) {
        // Transform version 100 -> 300 es (Somewhat. Probably more stuff missing still).
        preamble =
                "#version 300 es\n" \
                "precision mediump float;\n" \
                "#define OPENGL_ES\n" \
                "#define OPENGL_ES3\n" \
                "\n" \
                "#define attribute in\n" \
                "#define varying ouy\n\n";
    }
#else

    if (outputMajorVersion == 2) {
        preamble =
                "#version 120\n" \
                "define lowp\n" \
                "define mediump\n" \
                "define highp\n";

    } else if (outputMajorVersion == 3) {
        // Transform version 120 -> 130 (Somewhat. Probably more stuff missing still).
        preamble =
                "#version 130\n" \
                "#define lowp\n" \
                "#define mediump\n" \
                "#define highp\n" \
                "\n" \
                "#define attribute in\n" \
                "#define varying out\n\n";

    } else if (outputMajorVersion >= 4) {
        // Transform version 120 -> 130 (Somewhat. Probably more stuff missing still).
        preamble =
                "#version 400 core\n" \
                "#define lowp\n" \
                "#define mediump\n" \
                "#define highp\n" \
                "\n" \
                "#define attribute in\n" \
                "#define varying out\n\n";
    }
#endif

    int outputSize = sizeof(char) * (strlen(preamble) + strlen(shaderString) + 1);
    output = (char*) malloc(sizeof(char) * outputSize);
    if (output) {
        strcpy(output, preamble);
        strcat(output, shaderString);
    }
    return output;
}

OKIMPLGL_DEF char* okgl_convertFragmentShader(const char* shaderString,
                                              int outputMajorVersion,
                                              int outputMinorVersion) {
    OKIMPLGL_UNUSED(outputMinorVersion);

    char* preamble = "";
    char* output;

    if (!shaderString) {
        return NULL;
    }

    // TODO: better mappings for different versions.

#if OKGL_OPENGL_ES
    if (outputMajorVersion == 2) {
        preamble =
                "#version 100\n" \
                "precision mediump float;\n" \
                "#define OPENGL_ES\n" \
                "#define OPENGL_ES2\n";

    } else if (outputMajorVersion >= 3) {
        // Transform version 100 -> 300 es (Somewhat. Probably more stuff missing still).
        preamble =
                "#version 300 es\n" \
                "precision mediump float;\n" \
                "#define OPENGL_ES\n" \
                "#define OPENGL_ES3\n" \
                "\n" \
                "out highp vec4 fragmentOut;\n" \
                "\n" \
                "#define varying in\n" \
                "#define texture2D(a, b) texture(a, b)\n" \
                "#define textureCube(a, b) texture(a, b)\n" \
                "#define gl_FragColor fragmentOut\n";
    }
#else

    if (outputMajorVersion == 2) {
        preamble =
                "#version 120\n" \
                "define lowp\n" \
                "define mediump\n" \
                "define highp\n";

    } else if (outputMajorVersion == 3) {
        // Transform version 120 -> 130 (Somewhat. Probably more stuff missing still).
        preamble =
                "#version 130\n" \
                "#define lowp\n" \
                "#define mediump\n" \
                "#define highp\n" \
                "\n" \
                "out vec4 fragmentOut;\n" \
                "\n" \
                "#define varying in\n" \
                "#define texture2D(a, b) texture(a, b)\n" \
                "#define textureCube(a, b) texture(a, b)\n" \
                "#define gl_FragColor fragmentOut\n\n";

    } else if (outputMajorVersion >= 4) {
        // Transform version 120 -> 400 (Somewhat. Probably more stuff missing still).
        preamble =
                "#version 400 core\n" \
                "#define lowp\n" \
                "#define mediump\n" \
                "#define highp\n" \
                "\n" \
                "out vec4 fragmentOut;\n" \
                "\n" \
                "#define varying in\n" \
                "#define texture2D(a, b) texture(a, b)\n" \
                "#define textureCube(a, b) texture(a, b)\n" \
                "#define gl_FragColor fragmentOut\n\n";
    }
#endif

    int outputSize = sizeof(char) * (strlen(preamble) + strlen(shaderString) + 1);
    output = (char*) malloc(sizeof(char) * outputSize);
    if (output) {
        strcpy(output, preamble);
        strcat(output, shaderString);
    }
    return output;
}

// Object is either a shader or a linked program. Operation should be GL_COMPILE_STATUS or GL_LINK_STATUS.
OKIMPLGL_DEF_INTERNAL GLboolean okimplgl__checkStatus(GLuint object, GLenum operation) {
    GLint operationStatus;

    if (operation == GL_COMPILE_STATUS) {
        glGetShaderiv(object, operation, &operationStatus);
    } else {
        glGetProgramiv(object, operation, &operationStatus);
    }

    if (!operationStatus) {
        okimplgl__logError(OKIMPLGL_LOG_TAG, "Error compiling shader");

        GLint infoLogLength = 0;
        if (operation == GL_COMPILE_STATUS) {
            glGetShaderiv(object, GL_INFO_LOG_LENGTH, &infoLogLength);
        } else {
            glGetProgramiv(object, GL_INFO_LOG_LENGTH, &infoLogLength);
        }

        if (infoLogLength > 1) {
            char* infoLog = malloc(sizeof(char) * infoLogLength);
            if (operation == GL_COMPILE_STATUS) {
                glGetShaderInfoLog(object, infoLogLength, NULL, infoLog);
            } else {
                glGetProgramInfoLog(object, infoLogLength, NULL, infoLog);
            }
            okimplgl__logError(OKIMPLGL_LOG_TAG, "\n%s", infoLog);

            free(infoLog);
        }
        return GL_FALSE;
    }

    return GL_TRUE;
}

OKIMPLGL_DEF GLuint okgl_createShader(const char* shaderString, GLenum shaderType) {
    GLuint shader;

    shader = glCreateShader(shaderType);
    OKGL_CHECK_ERROR();
    if (!shader) {
        return 0;
    }

    glShaderSource(shader, 1, &shaderString, NULL);
    glCompileShader(shader);
    if (!okimplgl__checkStatus(shader, GL_COMPILE_STATUS)) {

#ifdef DEBUG
        // Print the whole problem shader with line numbering.
        okimplgl__logInfo(OKIMPLGL_LOG_TAG, "SHADER:");
        char* shaderStringTmp = malloc(sizeof(char) * (strlen(shaderString) + 1));
        strcpy(shaderStringTmp, shaderString);

        char* line = strtok(shaderStringTmp, "\n");
        int lineNumber = 1;
        while (line != NULL) {
            okimplgl__logInfo(OKIMPLGL_LOG_TAG, "%4d| %s", lineNumber, line);
            line = strtok(NULL, "\n");
            lineNumber++;
        }
        free(shaderStringTmp);
#endif

        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

OKIMPLGL_DEF GLuint okgl_linkProgram(const char* vertexShaderString, const char* fragmentShaderString) {
    GLuint program;
    GLint glMajorVersion, glMinorVersion;
    GLuint vertexShader = 0;
    GLuint fragmentShader = 0;

    program = glCreateProgram();
    if (!program) {
        return 0;
    }

    okgl_getVersion(&glMajorVersion, &glMinorVersion);

    char* vertexShaderStringFinal = okgl_convertVertexShader(vertexShaderString,
                                                             glMajorVersion,
                                                             glMinorVersion);
    if (vertexShaderStringFinal) {
        vertexShader = okgl_createShader(vertexShaderStringFinal, GL_VERTEX_SHADER);
        free(vertexShaderStringFinal);
    }

    char* fragmentShaderStringFinal = okgl_convertFragmentShader(fragmentShaderString,
                                                                 glMajorVersion,
                                                                 glMinorVersion);
    if (fragmentShaderStringFinal) {
        fragmentShader = okgl_createShader(fragmentShaderStringFinal, GL_FRAGMENT_SHADER);
        free(fragmentShaderStringFinal);
    }

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program);
    OKGL_CHECK_ERROR_ALWAYS();

    glDetachShader(program, vertexShader);
    glDetachShader(program, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    if (!okimplgl__checkStatus(program, GL_LINK_STATUS)) {
        glDeleteProgram(program);
        return 0;
    }

    return program;
}

#endif // ifdef OKGL_HELPER_IMPLEMENTATION

#ifdef __cplusplus
}
#endif

#endif // ifndef OKWRAPPER_OKGL_HELPER_H_
