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
 * This is a header only library containing some very basic OpenGL helper
 * functions. E.g. for creating shaders.
 *
 *
 * To build the implementation OKGL_IMPLEMENTATION needs to
 * be defined in exactly one compilation unit before including this file:
 *
 * #define OKGL_IMPLEMENTATION
 * #include "ok_gl_helper/ok_gl_helper.h"
 *
 * Note that you need to have the define before the file might be included from other included files too, because the
 * include guard prevents including later with the implementation. We are not using a separate guard for the
 * implementation here to take advantage of any include guard optimizations by the compiler
 * (https://gcc.gnu.org/onlinedocs/cppinternals/Guard-Macros.html)
 */

#ifndef OK_GL_HELPER_H_
#define OK_GL_HELPER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "ok_platform_wrapper/ok_opengl.h"

#ifdef OKGL_STATIC
#define OKGL_DEF static
#else
#define OKGL_DEF extern
#endif

#ifdef DEBUG
#define OKGL_CHECK_ERROR(...) okgl_checkErrorDebug(__FILE__, __LINE__)
#define OKGL_CHECK_ERROR_ALWAYS(...) okgl_checkErrorDebug(__FILE__, __LINE__)
#else
#define OKGL_CHECK_ERROR(...)
#define OKGL_CHECK_ERROR_ALWAYS(...) okgl_checkError()
#endif

OKGL_DEF unsigned int okgl_checkError();
OKGL_DEF unsigned int okgl_checkErrorDebug(const char* file, int line);
OKGL_DEF const char* okgl_getErrorString(unsigned int error);

OKGL_DEF GLint okgl_getInt(GLenum pname);
OKGL_DEF const char* okgl_getVersion(GLint* glMajorVersion, GLint* glMinorVersion);
OKGL_DEF void okgl_logInfo();

OKGL_DEF void okgl_matrixSetTranslation(GLfloat* matrix, GLfloat x, GLfloat y, GLfloat z);
OKGL_DEF void okgl_matrixSetscale(GLfloat* matrix, GLfloat x, GLfloat y, GLfloat z);
OKGL_DEF void okgl_unitMatrix(GLfloat* matrix);

OKGL_DEF void okgl_projection2d(GLfloat* projectionMatrix, GLfloat left, GLfloat right, GLfloat bottom, GLfloat top);

OKGL_DEF GLuint okgl_createShader(const char* shaderString, GLenum shaderType);
OKGL_DEF GLuint okgl_linkProgram(const char* vertexShaderString, const char* fragmentShaderString);


/*
 * ----------------------------------------------------------------------------
 * Implementation starts here.
 * ----------------------------------------------------------------------------
 */

#ifdef OKGL_IMPLEMENTATION

#include "ok_platform_wrapper/ok_log.h"

#define OKGL_DEF_INTERNAL static

#define OKGL_LOG_TAG "ok_gl_helper"

OKGL_DEF unsigned int okgl_checkError() {
    GLenum error = glGetError();
    if (error) {
        oklog_e(OKGL_LOG_TAG, "glGetError(%d)\n", error);
    }

    return error;
}

OKGL_DEF unsigned int okgl_checkErrorDebug(const char* file, int line) {
    GLenum error = glGetError();
    if (error) {
        oklog_e(OKGL_LOG_TAG, "glGetError(%d): %s @ %s line %d\n", error, okgl_getErrorString(error), file, line);
    }

    return error;
}

OKGL_DEF const char* okgl_getErrorString(unsigned int error) {
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

OKGL_DEF GLint okgl_getInt(GLenum pname) {
    GLint value;
    glGetIntegerv(pname, &value);
    return value;
}

OKGL_DEF const char* okgl_getVersion(GLint* glMajorVersion, GLint* glMinorVersion) {

    const char* version = (const char*) glGetString(GL_VERSION);

#if defined(OK_OPENGL_ES) && !defined(GL_MAJOR_VERSION) || !defined(GL_MINOR_VERSION)
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

OKGL_DEF void okgl_logInfo() {
    GLint glMajorVersion;
    GLint glMinorVersion;
    const char* version;

    version = okgl_getVersion(&glMajorVersion, &glMinorVersion);

#ifdef OK_OPENGL_ES
    oklog_i(OKGL_LOG_TAG, "OpenGL ES %d.%d:", glMajorVersion, glMinorVersion);
#else
    oklog_i(OKGL_LOG_TAG, "OpenGL %d.%d:", glMajorVersion, glMinorVersion);
#endif

    oklog_i(OKGL_LOG_TAG, "  version: %s", version);
    oklog_i(OKGL_LOG_TAG, "  renderer: %s", glGetString(GL_RENDERER));
    oklog_i(OKGL_LOG_TAG, "  vendor: %s", glGetString(GL_VENDOR));
    oklog_i(OKGL_LOG_TAG, "  glsl: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    oklog_i(OKGL_LOG_TAG, "  multisampling: buffers=%d samples=%d\n", okgl_getInt(GL_SAMPLE_BUFFERS), okgl_getInt(GL_SAMPLES));
    oklog_i(OKGL_LOG_TAG, "  texture image units: %d\n", okgl_getInt(GL_MAX_TEXTURE_IMAGE_UNITS));
    oklog_i(OKGL_LOG_TAG, "  max texture size: %d\n", okgl_getInt(GL_MAX_TEXTURE_SIZE));
    oklog_i(OKGL_LOG_TAG, "  max vertex attribs: %d\n", okgl_getInt(GL_MAX_VERTEX_ATTRIBS));

    if (glMajorVersion < 4) {
        oklog_i(OKGL_LOG_TAG, "  framebuffer: RGB %d%d%d alpha: %d depth: %d stencil: %d",
                okgl_getInt(GL_RED_BITS), okgl_getInt(GL_GREEN_BITS), okgl_getInt(GL_BLUE_BITS),
                okgl_getInt(GL_ALPHA_BITS), okgl_getInt(GL_DEPTH_BITS), okgl_getInt(GL_STENCIL_BITS));
    }

    OKGL_CHECK_ERROR();

    {
        GLint supportedTextureFormatCount = okgl_getInt(GL_NUM_COMPRESSED_TEXTURE_FORMATS);
        GLint* supportedFormats = malloc(sizeof(GLint) * supportedTextureFormatCount);

        glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS, supportedFormats);
        oklog_i(OKGL_LOG_TAG, "  some supported texture compression formats:\n");
        for (int i = 0; i < supportedTextureFormatCount; ++i) {
            switch (supportedFormats[i]) {
                case 0x8D64: // GL_ETC1_RGB8_OES
                    oklog_i(OKGL_LOG_TAG, "    ETC1\n");
                    break;

                case 0x9278: // GL_COMPRESSED_RGBA8_ETC2_EAC
                    oklog_i(OKGL_LOG_TAG, "    ETC2\n");
                    break;

                case 0x93B0: // GL_COMPRESSED_RGBA_ASTC_4x4_KHR
                    oklog_i(OKGL_LOG_TAG, "    ASTC\n");
                    break;

                case 0x8C00: // GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG
                    oklog_i(OKGL_LOG_TAG, "    PVRTC1\n");
                    break;

                case 0x9137: // GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG
                    oklog_i(OKGL_LOG_TAG, "    PVRTC2\n");
                    break;

                default:
                    break;
            }

            //oklog_i(SIMPLEGL_LOG_TAG, "  0x%X\n", supportedFormats[i]);
        }

        free(supportedFormats);
        OKGL_CHECK_ERROR();
    }
}

OKGL_DEF void okgl_unitMatrix(GLfloat* matrix) {
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

OKGL_DEF void okgl_matrixSetTranslation(GLfloat* matrix, GLfloat x, GLfloat y, GLfloat z) {
    matrix[12] = x;
    matrix[13] = y;
    matrix[14] = z;
}
OKGL_DEF void okgl_matrixSetscale(GLfloat* matrix, GLfloat x, GLfloat y, GLfloat z) {
    matrix[0] = x;
    matrix[5] = y;
    matrix[10] = z;
}

OKGL_DEF void okgl_projection2d(GLfloat* projectionMatrix, GLfloat left, GLfloat right, GLfloat bottom, GLfloat top) {
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

/*
 * With some minor changes OpenGL ES 2 shaders can be converted to ES 3 / gl3 core shaders.
 * This way the same shader can be used on multiple different platforms.
 *
 * Needs to be freed by the called.
 */
OKGL_DEF char* okgl_convertVertexShader(const char* shaderString,
                                        int outputMajorVersion,
                                        int outputMinorVersion) {
    char* preamble = "";
    char* output;

    if (!shaderString) {
        return NULL;
    }

    // TODO: better mappings for different versions.

#ifdef OK_OPENGL_ES
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

OKGL_DEF char* okgl_convertFragmentShader(const char* shaderString,
                                          int outputMajorVersion,
                                          int outputMinorVersion) {
    char* preamble = "";
    char* output;

    if (!shaderString) {
        return NULL;
    }

    // TODO: better mappings for different versions.

#ifdef OK_OPENGL_ES
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

/* Object is either a shader or a linked program. Operation should be GL_COMPILE_STATUS or GL_LINK_STATUS. */
OKGL_DEF_INTERNAL GLboolean okgl__checkStatus(GLuint object, GLenum operation) {
    GLint operationStatus;

    if (operation == GL_COMPILE_STATUS) {
        glGetShaderiv(object, operation, &operationStatus);
    } else {
        glGetProgramiv(object, operation, &operationStatus);
    }

    if (!operationStatus) {
        oklog_e(OKGL_LOG_TAG, "Error compiling shader");

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
            oklog_e(OKGL_LOG_TAG, "\n%s", infoLog);

            free(infoLog);
        }
        return GL_FALSE;
    }

    return GL_TRUE;
}

GLuint okgl_createShader(const char* shaderString, GLenum shaderType) {
    GLuint shader;

    shader = glCreateShader(shaderType);
    OKGL_CHECK_ERROR();
    if (!shader) {
        return 0;
    }

    glShaderSource(shader, 1, &shaderString, NULL);
    glCompileShader(shader);
    if (!okgl__checkStatus(shader, GL_COMPILE_STATUS)) {

#ifdef DEBUG
        // Print the whole problem shader with line numbering.
        oklog_i(OKGL_LOG_TAG, "SHADER:\n");
        char* shaderStringTmp = malloc(sizeof(char) * (strlen(shaderString) + 1));
        strcpy(shaderStringTmp, shaderString);

        char* line = strtok(shaderStringTmp, "\n");
        int lineNumber = 1;
        while (line != NULL) {
            oklog_i(OKGL_LOG_TAG, "%4d| %s\n", lineNumber, line);
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

GLuint okgl_linkProgram(const char* vertexShaderString, const char* fragmentShaderString) {
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

    if (!okgl__checkStatus(program, GL_LINK_STATUS)) {
        glDeleteProgram(program);
        return 0;
    }

    return program;
}

#endif /* ifdef OKGL_IMPLEMENTATION */

#ifdef __cplusplus
}
#endif

#endif /* ifndef OK_GL_HELPER_H_ */
