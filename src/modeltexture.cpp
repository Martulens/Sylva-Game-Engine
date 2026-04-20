/**
 * @file modeltexture.cpp
 * @brief Implementation of the ModelTexture and TexturePack classes for texture handling.
 */

#include "modeltexture.h"
#include "sylva/sylva.h"
#include <iostream>

 /**
  * @brief Constructs a ModelTexture using multiple texture maps (PBR-style).
  *
  * @param baseColor Path to the base color texture.
  * @param normal Path to the normal map texture.
  * @param roughness Path to the roughness map texture.
  * @param heightTex Path to the height map texture.
  * @param emissive Path to the emissive map texture.
  */
ModelTexture::ModelTexture(const std::string& baseColor,
    const std::string& normal,
    const std::string& roughness,
    const std::string& heightTex,
    const std::string& emissive) {
    baseColorID = sylva::createTexture(baseColor);
    normalID    = sylva::createTexture(normal);
    roughnessID = sylva::createTexture(roughness);
    heightID    = sylva::createTexture(heightTex);
    emissiveID  = sylva::createTexture(emissive);

    if (baseColorID == 0) {
        std::cerr << "Failed to load base color texture: " << baseColor << std::endl;
        exit(EXIT_FAILURE);
    }

    struct { GLuint id; const std::string& path; const char* kind; } pbrMaps[] = {
        { normalID,    normal,    "normal"    },
        { roughnessID, roughness, "roughness" },
        { heightID,    heightTex, "height"    },
        { emissiveID,  emissive,  "emissive"  },
    };
    for (const auto& m : pbrMaps) {
        if (m.id == 0) {
            std::cerr << "Warning: " << m.kind << " map failed to load: " << m.path << std::endl;
        }
    }

    glBindTexture(GL_TEXTURE_2D, baseColorID);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
    glBindTexture(GL_TEXTURE_2D, 0);
}

/**
 * @brief Constructs a ModelTexture using a single texture file (e.g., for simple objects).
 *
 * @param fileName Path to the texture file.
 */
ModelTexture::ModelTexture(const std::string& fileName) {
    textureID = sylva::createTexture(fileName);

    if (textureID == 0) {
        std::cerr << "Failed to load texture: " << fileName << std::endl;
        exit(EXIT_FAILURE);
    }

    glBindTexture(GL_TEXTURE_2D, textureID);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
    glBindTexture(GL_TEXTURE_2D, 0);
}

/**
 * @brief Constructs a TexturePack used for terrain texturing with up to four different textures.
 *
 * @param s1 Texture for the background (e.g., grass).
 * @param s2 Texture for red channel influence (e.g., sand).
 * @param s3 Texture for green channel influence (e.g., water).
 * @param s4 Texture for blue channel influence (e.g., tiles).
 */
TexturePack::TexturePack(ModelTexture& s1, ModelTexture& s2, ModelTexture& s3, ModelTexture& s4) {
    backTexture = s1;
    rTexture = s2;
    gTexture = s3;
    bTexture = s4;
}
