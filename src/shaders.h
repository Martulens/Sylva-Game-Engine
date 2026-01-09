/**
 * @file shaders.h
 * @brief Declares the ShaderProgram class for managing GLSL shader programs and uniforms.
 */

#pragma once
#include "lib.h"

 /**
  * @class ShaderProgram
  * @brief Encapsulates OpenGL shader program setup, uniform locations, and attribute bindings.
  */
class ShaderProgram {
public:
    GLuint program;                     ///< OpenGL shader program ID.
    GLint posLocation;                 ///< Location of vertex position attribute.
    GLint colorLocation;               ///< Location of vertex color attribute.
    GLint PVMLocation;                 ///< Location of combined PVM matrix (optional/legacy).

    // Vertex shader uniform locations
    GLint transformationMatrix;        ///< Model transformation matrix.
    GLint projectionMatrix;            ///< Projection matrix.
    GLint viewMatrix;                  ///< View matrix.

    GLint density;                     ///< Fog density.
    GLint gradient;                    ///< Fog gradient.

    // Fragment shader uniform locations
    GLint shineDamper;                 ///< Controls specular highlight size.
    GLint reflectivity;               ///< Controls specular reflectivity.
    GLint lightPosition;              ///< Position of light source.
    GLint lightColour;                ///< Color of light.
    GLint lightDirection;             ///< Direction of directional light.
    GLint skyColor;                   ///< Sky/fog color.

    // Texture sampler uniforms
    GLint backTexture;
    GLint rTexture;
    GLint bTexture;
    GLint gTexture;
    GLint blendMap;

    // Flashlight uniforms
    GLint flashlightOn;
    GLint flashlightPos;
    GLint flashlightDir;
    GLint cutoffAngle;

    GLint screenCoordLocation;        ///< Screen coordinate input (used for skybox/fullscreen shaders).

    ShaderProgram() = default;

    /**
     * @brief Constructs and initializes shader program from vertex and fragment shader file paths.
     * @param vert Path to vertex shader.
     * @param frag Path to fragment shader.
     */
    ShaderProgram(const std::string& vert, const std::string& frag);

    /**
     * @brief Destroys the shader program and associated resources.
     */
    ~ShaderProgram();

    /**
     * @brief Loads a float uniform value.
     * @param location Location of the uniform.
     * @param value Float value to load.
     */
    void loadFloat(GLint location, float value);

    /**
     * @brief Initializes the shader program with the given vertex and fragment shader files.
     * @param vert Path to vertex shader file.
     * @param frag Path to fragment shader file.
     */
    void init(const std::string& vert, const std::string& frag);

    /**
     * @brief Retrieves all sampler uniform locations.
     */
    void getSamplerUniforms();

    /**
     * @brief Retrieves all general uniform locations used in the shaders.
     */
    void getAllUniforms();

    /**
     * @brief Binds attribute locations for the shader program.
     */
    void bindAttributes();
};
