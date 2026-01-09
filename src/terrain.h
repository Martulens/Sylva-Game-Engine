/**
 * @file terrain.h
 * @brief Declares the Terrain class for procedural terrain generation, rendering, and texturing.
 */

#ifndef TERRAIN_H
#define TERRAIN_H

#include "meshgeometry.h"
#include "modeltexture.h"
#include "camera.h"
#include "data.h"
#include "shaders.h"

 /**
  * @class Terrain
  * @brief Represents a segment of procedurally generated terrain with texture blending and height queries.
  */
class Terrain {
private:
    float x; ///< World X-coordinate of the terrain origin.
    float z; ///< World Z-coordinate of the terrain origin.

    std::vector<std::vector<float>> heightMap; ///< Stores height values for terrain vertices.

public:
    ShaderProgram* shader;       ///< Shader used for terrain rendering.
    MeshGeometry* geometry;      ///< Geometry data for the terrain mesh.
    TexturePack texture;         ///< Collection of terrain textures for multi-layer blending.
    ModelTexture* blendMap;      ///< Blend map determining texture layer blending.

    Terrain() = default;

    /**
     * @brief Constructs and generates a new terrain patch.
     * @param gridX X-grid coordinate.
     * @param gridZ Z-grid coordinate.
     * @param texturepack Textures to be used on the terrain.
     * @param blend Blend map texture for texture blending.
     */
    Terrain(int gridX, int gridZ, TexturePack texturepack, ModelTexture* blend);

    /**
     * @brief Destroys the terrain and releases GPU resources.
     */
    ~Terrain();

    /**
     * @brief Retrieves the height of the terrain at a given world-space location.
     * @param x World X-coordinate.
     * @param y World Z-coordinate.
     * @return Interpolated height at that location.
     */
    float getHeight(float x, float y) const;

    /**
     * @brief Computes a procedural height value using Perlin noise.
     * @param worldX World X-coordinate.
     * @param worldZ World Z-coordinate.
     * @return Procedural height value.
     */
    float proceduralHeight(float worldX, float worldZ);

    /**
     * @brief Generates the terrain geometry using flat grid and procedural height data.
     */
    void generateFlat();

    /**
     * @brief Draws the terrain using its shader and geometry.
     */
    void draw();

private:
    /**
     * @brief Binds the VAO and sets up rendering state.
     */
    void bind();

    /**
     * @brief Loads and uploads the terrain's model transformation matrix.
     */
    void loadModelMatrix();

    /**
     * @brief Unbinds the VAO and resets rendering state.
     */
    void unbind();
};

#endif // TERRAIN_H
