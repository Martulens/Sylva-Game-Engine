/**
 * @file modeltexture.h
 * @brief Contains texture data structures and classes used for rendering materials.
 */

#ifndef MODELTEXTURE_H
#define MODELTEXTURE_H
#include <string>
#include <GL/glew.h>

/**
  * @class ModelTexture
  * @brief Represents texture maps used for rendering materials of a 3D model.
  */
class ModelTexture {
public:
    GLuint baseColorID = 0;  ///< Base color texture ID.
    GLuint normalID = 0;     ///< Normal map texture ID.
    GLuint roughnessID = 0;  ///< Roughness texture ID.
    GLuint heightID = 0;     ///< Height/displacement texture ID.
    GLuint emissiveID = 0;   ///< Emissive texture ID.

    GLuint textureID;        ///< Single texture ID (used for simpler models).
    int width = 0;           ///< Width of the texture.
    int height = 0;          ///< Height of the texture.

    float shineDamper = 1.0;     ///< Dampening factor for specular highlights.
    float reflectivity = 0.0;    ///< Reflectivity factor for specular highlights.

    ModelTexture() = default;

    /**
     * @brief Constructs a ModelTexture from a single file.
     * @param fileName Path to the texture file.
     */
    ModelTexture(const std::string& fileName);

    /**
     * @brief Constructs a PBR material texture using separate texture maps.
     * @param baseColor Path to base color texture.
     * @param normal Path to normal map.
     * @param roughness Path to roughness texture.
     * @param height Path to height/displacement texture.
     * @param emissive Path to emissive texture.
     */
    ModelTexture(const std::string& baseColor,
        const std::string& normal,
        const std::string& roughness,
        const std::string& height,
        const std::string& emissive);
};

/**
 * @class TexturePack
 * @brief Groups multiple textures for different terrain layers.
 */
class TexturePack {
public:
    ModelTexture backTexture;  ///< Background layer texture.
    ModelTexture rTexture;     ///< Red layer texture.
    ModelTexture gTexture;     ///< Green layer texture.
    ModelTexture bTexture;     ///< Blue layer texture.

    TexturePack() = default;

    /**
     * @brief Constructs a texture pack from four ModelTextures.
     * @param s1 Back texture.
     * @param s2 Red texture.
     * @param s3 Green texture.
     * @param s4 Blue texture.
     */
    TexturePack(ModelTexture& s1, ModelTexture& s2, ModelTexture& s3, ModelTexture& s4);
};

#endif // MODELTEXTURE_H
