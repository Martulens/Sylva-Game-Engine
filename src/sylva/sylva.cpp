#include "sylva/sylva.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

namespace sylva {

void dieWithError(const char* msg) {
    std::fprintf(stderr, "fatal: %s\n", msg);
    std::exit(EXIT_FAILURE);
}

float elapsedSeconds() {
    static const auto start = std::chrono::steady_clock::now();
    const auto now = std::chrono::steady_clock::now();
    return std::chrono::duration<float>(now - start).count();
}

void checkGlError(const char* file, int line) {
    GLenum err = glGetError();
    while (err != GL_NO_ERROR) {
        const char* name = "UNKNOWN";
        switch (err) {
            case GL_INVALID_ENUM:                  name = "GL_INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 name = "GL_INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             name = "GL_INVALID_OPERATION"; break;
            case GL_OUT_OF_MEMORY:                 name = "GL_OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: name = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
            default: break;
        }
        std::fprintf(stderr, "GL error %s at %s:%d\n", name, file, line);
        err = glGetError();
    }
}

static unsigned char* loadImageRgba(const std::string& fileName, int& w, int& h) {
    stbi_set_flip_vertically_on_load(1);
    int channels = 0;
    unsigned char* pixels = stbi_load(fileName.c_str(), &w, &h, &channels, STBI_rgb_alpha);
    if (!pixels) {
        std::fprintf(stderr, "failed to load '%s': %s\n",
                     fileName.c_str(), stbi_failure_reason());
    }
    return pixels;
}

GLuint createTexture(const std::string& fileName) {
    int w = 0, h = 0;
    unsigned char* pixels = loadImageRgba(fileName, w, h);
    if (!pixels) return 0;

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(pixels);
    return tex;
}

bool loadTexImage2D(const std::string& fileName, GLenum target) {
    int w = 0, h = 0;
    unsigned char* pixels = loadImageRgba(fileName, w, h);
    if (!pixels) return false;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexImage2D(target, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    stbi_image_free(pixels);
    return true;
}

static std::string readFile(const std::string& path) {
    std::ifstream f(path);
    if (!f) return {};
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

GLuint createShaderFromFile(GLenum stage, const std::string& fileName) {
    std::string source = readFile(fileName);
    if (source.empty()) {
        std::fprintf(stderr, "cannot open shader '%s'\n", fileName.c_str());
        return 0;
    }

    GLuint shader = glCreateShader(stage);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint ok = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLint logLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
        std::string log(static_cast<size_t>(logLen > 0 ? logLen : 1), '\0');
        glGetShaderInfoLog(shader, logLen, nullptr, log.data());
        std::fprintf(stderr, "shader compile failed for '%s':\n%s\n",
                     fileName.c_str(), log.c_str());
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint createProgram(const std::vector<GLuint>& shaders) {
    GLuint program = glCreateProgram();
    for (GLuint s : shaders) glAttachShader(program, s);
    glLinkProgram(program);

    GLint ok = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok) {
        GLint logLen = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
        std::string log(static_cast<size_t>(logLen > 0 ? logLen : 1), '\0');
        glGetProgramInfoLog(program, logLen, nullptr, log.data());
        std::fprintf(stderr, "program link failed:\n%s\n", log.c_str());
        glDeleteProgram(program);
        for (GLuint s : shaders) glDeleteShader(s);
        return 0;
    }

    for (GLuint s : shaders) {
        glDetachShader(program, s);
        glDeleteShader(s);
    }
    return program;
}

void deleteProgramAndShaders(GLuint program) {
    if (program == 0) return;
    GLint numAttached = 0;
    glGetProgramiv(program, GL_ATTACHED_SHADERS, &numAttached);
    if (numAttached > 0) {
        std::vector<GLuint> attached(static_cast<size_t>(numAttached));
        glGetAttachedShaders(program, numAttached, nullptr, attached.data());
        for (GLuint s : attached) {
            glDetachShader(program, s);
            glDeleteShader(s);
        }
    }
    glDeleteProgram(program);
}

} // namespace sylva
