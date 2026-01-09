/**
 * @file terrain.cpp
 * @brief Implements the Terrain class for procedural terrain generation and rendering.
 */

#include "terrain.h"
#include "data.h"
#include "camera.h"
#include "noise.h"

 /**
  * @brief Constructs a new Terrain object.
  * @param gridX Grid X position.
  * @param gridZ Grid Z position.
  * @param texturepack Set of textures for different terrain layers.
  * @param blend Pointer to the blend map texture.
  */
Terrain::Terrain(int gridX, int gridZ, TexturePack texturepack, ModelTexture* blend) {
    texture = texturepack;
    blendMap = blend;

    x = gridX * TERRAIN_SIZE;
    z = gridZ * TERRAIN_SIZE;

    geometry = new MeshGeometry();
    shader = new ShaderProgram("terrain.vert", "terrain.frag");
    generateFlat();
}

/**
 * @brief Gets the terrain height at a given world position using bilinear interpolation.
 * @param worldX World X coordinate.
 * @param worldZ World Z coordinate.
 * @return Interpolated terrain height.
 */
float Terrain::getHeight(float worldX, float worldZ) const {
    float localX = worldX - x;
    float localZ = worldZ - z;

    int gridX = (int)(localX / TERRAIN_SIZE * VERTEX_COUNT_TERRAIN);
    int gridZ = (int)(localZ / TERRAIN_SIZE * VERTEX_COUNT_TERRAIN);

    if (gridX < 0 || gridX >= heightMap.size() - 1 || gridZ < 0 || gridZ >= heightMap[0].size() - 1)
        return 0.0f;

    float fracX = (localX / TERRAIN_SIZE * VERTEX_COUNT_TERRAIN) - gridX;
    float fracZ = (localZ / TERRAIN_SIZE * VERTEX_COUNT_TERRAIN) - gridZ;

    float h1 = heightMap[gridZ][gridX];
    float h2 = heightMap[gridZ][gridX + 1];
    float h3 = heightMap[gridZ + 1][gridX];
    float h4 = heightMap[gridZ + 1][gridX + 1];

    float hA = h1 * (1 - fracX) + h2 * fracX;
    float hB = h3 * (1 - fracX) + h4 * fracX;

    return hA * (1 - fracZ) + hB * fracZ;
}

/**
 * @brief Uploads the model transformation matrix to the shader.
 */
void Terrain::loadModelMatrix() {
    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.0f, z));
    if (shader->transformationMatrix != -1) {
        glUniformMatrix4fv(shader->transformationMatrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));
    }
}

/**
 * @brief Binds the terrain geometry and enables required vertex attributes.
 */
void Terrain::bind() {
    glUseProgram(shader->program);
    glBindVertexArray(geometry->vao);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
}

/**
 * @brief Unbinds the terrain geometry and disables vertex attributes.
 */
void Terrain::unbind() {
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glBindVertexArray(0);
}

/**
 * @brief Draws the terrain mesh.
 */
void Terrain::draw() {
    bind();
    loadModelMatrix();
    glDrawElements(GL_TRIANGLES, geometry->numTriangles * 3, GL_UNSIGNED_INT, 0);
    unbind();
}

/**
 * @brief Computes procedural height using Perlin noise.
 * @param worldX X coordinate in world space.
 * @param worldZ Z coordinate in world space.
 * @return Procedural height value.
 */
float Terrain::proceduralHeight(float worldX, float worldZ) {
    float total = 0.0f;
    float frequency = 0.01f;
    float amplitude = 5.0f;
    float persistence = 0.5f;
    float lacunarity = 2.0f;
    int octaves = 4;

    for (int i = 0; i < octaves; ++i) {
        total += perlin(worldX * frequency, worldZ * frequency) * amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }

    return total;
}

/**
 * @brief Generates a flat terrain mesh with procedural height values.
 */
void Terrain::generateFlat() {
    int nVertices = VERTEX_COUNT_TERRAIN;
    float tSize = TERRAIN_SIZE;

    int count = nVertices * nVertices;
    std::vector<Vertex> verts(count);
    std::vector<unsigned int> indices(6 * (nVertices - 1) * (nVertices - 1));

    heightMap.resize(tSize, std::vector<float>(tSize));

    unsigned int vertexPointer = 0;
    for (int i = 0; i < nVertices; ++i) {
        for (int j = 0; j < nVertices; ++j) {
            Vertex v;
            float x = (float)j / (nVertices - 1) * tSize;
            float z = (float)i / (nVertices - 1) * tSize;

            float worldX = x + this->x;
            float worldZ = z + this->z;

            float y = proceduralHeight(worldX, worldZ);
            heightMap[i][j] = y;

            v.position = glm::vec3(x, y, z);
            v.normal = glm::vec3(0.0f, 1.0f, 0.0f);
            v.texCoord = glm::vec2((float)j / (nVertices - 1), (float)i / (nVertices - 1));
            v.color = glm::vec3(1.0f);

            verts[vertexPointer++] = v;
        }
    }

    unsigned int pointer = 0;
    for (int z = 0; z < nVertices - 1; ++z) {
        for (int x = 0; x < nVertices - 1; ++x) {
            int topLeft = (z * nVertices) + x;
            int topRight = topLeft + 1;
            int bottomLeft = ((z + 1) * nVertices) + x;
            int bottomRight = bottomLeft + 1;

            indices[pointer++] = topLeft;
            indices[pointer++] = bottomLeft;
            indices[pointer++] = topRight;
            indices[pointer++] = topRight;
            indices[pointer++] = bottomLeft;
            indices[pointer++] = bottomRight;
        }
    }

    geometry->vertices = verts;
    geometry->indices = indices;
    geometry->numTriangles = indices.size() / 3;
    geometry->loadVAO();
}

/**
 * @brief Terrain destructor that cleans up shader and geometry resources.
 */
Terrain::~Terrain() {
    delete shader;
    geometry->cleanupGeometry();
}
