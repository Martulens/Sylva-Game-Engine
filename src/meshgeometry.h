/**
 * @file meshgeometry.h
 * @brief Defines the MeshGeometry class for managing vertex data, bone structures, and mesh loading.
 */

#ifndef MESH_H
#define MESH_H

#include "glm/fwd.hpp"
#include "glm/matrix.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "data.h"

#include <map>
#include <iostream>
#include <GL/gl.h>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"

/**
  * @struct Vertex
  * @brief Structure representing a single vertex with position, normal, texture coordinate, color, and skinning data.
  */
struct Vertex {
    glm::vec3 position;   ///< Position of the vertex.
    glm::vec3 normal;     ///< Normal vector.
    glm::vec2 texCoord;   ///< Texture coordinates.
    glm::vec3 color;      ///< Vertex color (used optionally for debugging or shaders).

    int boneIDs[MAX_BONES_PER_VERTEX];    ///< Indices of bones influencing this vertex.
    float weights[MAX_BONES_PER_VERTEX];  ///< Weights corresponding to each bone ID.
};

/**
 * @class MeshGeometry
 * @brief Handles mesh loading, geometry data, bone animation, and rendering buffers.
 */
class MeshGeometry {
public:
    GLuint vbo; ///< Vertex Buffer Object.
    GLuint ebo; ///< Element Buffer Object.
    GLuint vao; ///< Vertex Array Object.

    float maxY = 0.0f;              ///< Highest Y value in the mesh.
    unsigned int numVertices;      ///< Number of vertices in the mesh.
    unsigned int numTriangles;     ///< Number of triangles in the mesh.
    float offsetToFeet = 0.0f;     ///< Offset to align mesh feet to origin.

    std::vector<unsigned int> indices;   ///< Index data for triangles.
    std::vector<Vertex> vertices;        ///< Vertex data.

    std::map<std::string, int> boneMapping;            ///< Maps bone names to indices.
    std::vector<glm::mat4> boneOffsets;                ///< Bone offset matrices.
    std::vector<glm::mat4> boneFinalTransforms;        ///< Final transformation matrices per bone.
    int numBones = 0;                                  ///< Number of bones used.

    const aiScene* scene = nullptr;                    ///< Assimp scene data.
    Assimp::Importer importer;                         ///< Assimp importer instance.
    std::map<std::string, unsigned int> animationNameToIndex; ///< Optional animation name mapping.

    MeshGeometry() = default;

    /**
     * @brief Constructs a mesh geometry from a file.
     * @param fileName Path to the mesh file.
     */
    MeshGeometry(std::string& fileName);

    /**
     * @brief Constructs a mesh geometry from a C-style file path.
     * @param fileName Path to the mesh file.
     */
    MeshGeometry(const char* fileName);

    /**
     * @brief Generates a triangle cross mesh.
     */
    void createCross();

    /**
     * @brief Generates a sphere mesh with defined resolution.
     */
    void generateSimpleSphereMesh();

    /**
     * @brief Processes bone hierarchy recursively.
     * @param node Pointer to the current node.
     * @param mat Transformation from parent.
     */
    void processNodeHierarchy(aiNode* node, const glm::mat4& mat);

    /**
     * @brief Loads a model and extracts geometry and animation data.
     * @param fileName Path to the model file.
     */
    void loadModel(std::string& fileName);

    /**
     * @brief Loads and binds the vertex array and buffers.
     */
    void loadVAO();

    /**
     * @brief Releases OpenGL buffer resources.
     */
    void cleanupGeometry();
};

#endif // MESH_H
