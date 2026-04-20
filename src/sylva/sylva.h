#ifndef SYLVA_SYLVA_H
#define SYLVA_SYLVA_H

#include <glad/gl.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#ifndef NDEBUG
    #define CHECK_GL_ERROR() ::sylva::checkGlError(__FILE__, __LINE__)
#else
    #define CHECK_GL_ERROR() ((void)0)
#endif

namespace sylva {

constexpr int OGL_VER_MAJOR = 3;
constexpr int OGL_VER_MINOR = 3;

[[noreturn]] void dieWithError(const char* msg);

GLuint createTexture(const std::string& fileName);
bool   loadTexImage2D(const std::string& fileName, GLenum target);

GLuint createShaderFromFile(GLenum stage, const std::string& fileName);
GLuint createProgram(const std::vector<GLuint>& shaders);
void   deleteProgramAndShaders(GLuint program);

float elapsedSeconds();
inline int elapsedMilliseconds() {
    return static_cast<int>(elapsedSeconds() * 1000.0f);
}

void checkGlError(const char* file, int line);

} // namespace sylva

#endif // SYLVA_SYLVA_H
