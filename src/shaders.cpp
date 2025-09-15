/**
 * @file shaders.cpp
 * @brief Implements the ShaderProgram class for handling GLSL shader programs.
 */

#include "shaders.h"
#include <iostream>

 /**
  * @brief Constructs a ShaderProgram by loading and linking given vertex and fragment shaders.
  * @param vert Path to the vertex shader file.
  * @param frag Path to the fragment shader file.
  */
ShaderProgram::ShaderProgram(const std::string& vert, const std::string& frag) {
    init(vert, frag);
}

/**
 * @brief Loads a float uniform value into the shader program.
 * @param location Location of the uniform variable.
 * @param value Float value to upload.
 */
void ShaderProgram::loadFloat(GLint location, float value) {
    glUniform1f(location, value);
}

/**
 * @brief Initializes the shader program by compiling shaders and linking them.
 * @param vert Path to the vertex shader.
 * @param frag Path to the fragment shader.
 */
void ShaderProgram::init(const std::string& vert, const std::string& frag) {
    std::vector<GLuint> shaderList;
    shaderList.push_back(pgr::createShaderFromFile(GL_VERTEX_SHADER, vert));
    shaderList.push_back(pgr::createShaderFromFile(GL_FRAGMENT_SHADER, frag));

    std::cout << vert << " " << frag << std::endl;
    program = pgr::createProgram(shaderList);

    getAllUniforms();
    getSamplerUniforms();
    bindAttributes();
}

/**
 * @brief Retrieves the locations of commonly used uniform variables.
 */
void ShaderProgram::getAllUniforms() {
    transformationMatrix = glGetUniformLocation(program, "transformationMatrix");
    projectionMatrix = glGetUniformLocation(program, "projectionMatrix");
    viewMatrix = glGetUniformLocation(program, "viewMatrix");

    density = glGetUniformLocation(program, "density");
    gradient = glGetUniformLocation(program, "gradient");

    shineDamper = glGetUniformLocation(program, "shineDamper");
    reflectivity = glGetUniformLocation(program, "reflectivity");

    lightPosition = glGetUniformLocation(program, "lightPosition");
    lightColour = glGetUniformLocation(program, "lightColour");
    lightDirection = glGetUniformLocation(program, "lightDirection");

    skyColor = glGetUniformLocation(program, "skyColor");

    flashlightOn = glGetUniformLocation(program, "flashlightOn");
    flashlightPos = glGetUniformLocation(program, "flashlightPos");
    flashlightDir = glGetUniformLocation(program, "flashlightDir");
    cutoffAngle = glGetUniformLocation(program, "cutoffAngle");
}

/**
 * @brief Retrieves the locations of texture sampler uniforms.
 */
void ShaderProgram::getSamplerUniforms() {
    backTexture = glGetUniformLocation(program, "backTexture");
    rTexture = glGetUniformLocation(program, "rTexture");
    gTexture = glGetUniformLocation(program, "gTexture");
    bTexture = glGetUniformLocation(program, "bTexture");
    blendMap = glGetUniformLocation(program, "blendMap");
}

/**
 * @brief Binds attribute indices to variable names used in the vertex shader.
 */
void ShaderProgram::bindAttributes() {
    glBindAttribLocation(program, 0, "position");
    glBindAttribLocation(program, 1, "texCoord");
    glBindAttribLocation(program, 2, "normal");
    glBindAttribLocation(program, 3, "color");
}

/**
 * @brief Destructor that deletes the shader program and associated shaders.
 */
ShaderProgram::~ShaderProgram() {
    pgr::deleteProgramAndShaders(program);
}
