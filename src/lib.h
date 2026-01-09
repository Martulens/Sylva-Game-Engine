/**
 * @file pgr.h
 * @brief OpenGL utility functions replacing the PGR framework.
 * 
 * Provides shader loading, texture loading, and OpenGL initialization utilities.
 */

#ifndef LIB_H
#define LIB_H

// OpenGL / GLEW
#include <GL/glew.h>
#include <GL/freeglut.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

// Assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// Standard library
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

/**
 * @brief Macro to check for OpenGL errors.
 * Prints file, line, and error description if an error occurred.
 */
#define CHECK_GL_ERROR() pgr::checkGLError(__FILE__, __LINE__)

namespace pgr {

    /** @brief Major OpenGL version required */
    const int OGL_VER_MAJOR = 3;
    /** @brief Minor OpenGL version required */
    const int OGL_VER_MINOR = 3;

    /**
     * @struct MeshData
     * @brief Basic mesh data structure for compatibility.
     */
    struct MeshData {
        std::vector<float> vertices;
        std::vector<float> normals;
        std::vector<float> texCoords;
        std::vector<unsigned int> indices;
        unsigned int numVertices;
        unsigned int numIndices;
    };

    /**
     * @brief Initializes GLEW and checks OpenGL version.
     * @param major Required major OpenGL version.
     * @param minor Required minor OpenGL version.
     * @return true if initialization succeeded, false otherwise.
     */
    bool initialize(int major = OGL_VER_MAJOR, int minor = OGL_VER_MINOR);

    /**
     * @brief Prints an error message and terminates the application.
     * @param message Error message to display.
     */
    void dieWithError(const std::string& message);

    /**
     * @brief Checks for OpenGL errors and prints them.
     * @param file Source file name.
     * @param line Line number.
     */
    void checkGLError(const char* file, int line);

    /**
     * @brief Reads the contents of a file into a string.
     * @param filename Path to the file.
     * @return File contents as a string.
     */
    std::string readFile(const std::string& filename);

    /**
     * @brief Creates and compiles a shader from a file.
     * @param shaderType Type of shader (GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, etc.).
     * @param filename Path to the shader source file.
     * @return Compiled shader ID, or 0 on failure.
     */
    GLuint createShaderFromFile(GLenum shaderType, const std::string& filename);

    /**
     * @brief Creates and compiles a shader from source code.
     * @param shaderType Type of shader.
     * @param source Shader source code.
     * @return Compiled shader ID, or 0 on failure.
     */
    GLuint createShaderFromSource(GLenum shaderType, const std::string& source);

    /**
     * @brief Links shaders into a program.
     * @param shaders Vector of compiled shader IDs.
     * @return Linked program ID, or 0 on failure.
     */
    GLuint createProgram(const std::vector<GLuint>& shaders);

    /**
     * @brief Deletes a shader program and its attached shaders.
     * @param program Program ID to delete.
     */
    void deleteProgramAndShaders(GLuint program);

    /**
     * @brief Loads a texture from a file.
     * @param filename Path to the texture file.
     * @return OpenGL texture ID, or 0 on failure.
     */
    GLuint createTexture(const std::string& filename);

    /**
     * @brief Loads a texture image into a specific OpenGL texture target.
     * @param filename Path to the texture file.
     * @param target OpenGL texture target (e.g., GL_TEXTURE_2D, GL_TEXTURE_CUBE_MAP_POSITIVE_X).
     * @return true if successful, false otherwise.
     */
    bool loadTexImage2D(const std::string& filename, GLenum target);

} // namespace pgr

#endif // LIB_H