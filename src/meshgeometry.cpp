/**
 * @file meshgeometry.cpp
 * @brief Implementation of the MeshGeometry class for handling model meshes.
 */

#include "meshgeometry.h"
#include "data.h"

#include <assimp/config.h>
#include <algorithm>

namespace {

const aiNode* findNodeWithMesh(const aiNode* node, unsigned int meshIndex) {
    if (!node) return nullptr;
    for (unsigned int i = 0; i < node->mNumMeshes; ++i)
        if (node->mMeshes[i] == meshIndex) return node;
    for (unsigned int i = 0; i < node->mNumChildren; ++i)
        if (const aiNode* found = findNodeWithMesh(node->mChildren[i], meshIndex))
            return found;
    return nullptr;
}

glm::mat4 nodeWorldTransform(const aiNode* node) {
    if (!node) return glm::mat4(1.0f);
    glm::mat4 local = glm::transpose(glm::make_mat4(&node->mTransformation.a1));
    if (node->mParent) return nodeWorldTransform(node->mParent) * local;
    return local;
}

} // namespace

/**
 * @brief Constructor that loads a model from a file.
 * @param fileName Path to the model file.
 */
MeshGeometry::MeshGeometry(std::string& fileName) {
    loadModel(fileName);
}

/**
 * @brief Constructor that loads a model from a file (C-string overload).
 * @param fileName Path to the model file.
 */
MeshGeometry::MeshGeometry(const char* fileName) {
    std::string inputPath(fileName);
    loadModel(inputPath);
}


/**
 * @brief Loads a model file and extracts geometry, bones, and animation data.
 * @param fileName Path to the model file.
 */
void MeshGeometry::loadModel(std::string& fileName) {
    importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
    importer.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, 4);

    scene = importer.ReadFile(fileName,
        aiProcess_Triangulate | aiProcess_FlipUVs |
        aiProcess_GenNormals | aiProcess_LimitBoneWeights);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }

    vertices.clear();
    indices.clear();

    unsigned int vertexOffset = 0;
    std::vector<int> bonesAssigned;

    for (unsigned int m = 0; m < scene->mNumMeshes; ++m) {
        aiMesh* mesh = scene->mMeshes[m];

        for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
            Vertex vertex;
            vertex.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
            vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
            vertex.texCoord = mesh->HasTextureCoords(0)
                ? glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y)
                : glm::vec2(0.0f);

            vertex.color = glm::vec3(1.0f);
            maxY = std::max(maxY, mesh->mVertices[i].y);

            std::fill(std::begin(vertex.boneIDs), std::end(vertex.boneIDs), 0);
            std::fill(std::begin(vertex.weights), std::end(vertex.weights), 0.0f);

            vertices.push_back(vertex);
        }

        bonesAssigned.resize(vertices.size(), 0);

        for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; ++j) {
                indices.push_back(face.mIndices[j] + vertexOffset);
            }
        }

        if (mesh->HasBones()) {
            for (unsigned int b = 0; b < mesh->mNumBones; ++b) {
                std::string boneName(mesh->mBones[b]->mName.C_Str());

                int boneIndex;
                if (boneMapping.find(boneName) == boneMapping.end()) {
                    boneIndex = numBones++;
                    boneMapping[boneName] = boneIndex;
                    glm::mat4 offset = glm::transpose(glm::make_mat4(&mesh->mBones[b]->mOffsetMatrix.a1));
                    boneOffsets.push_back(offset);
                    boneFinalTransforms.push_back(glm::mat4(1.0f));
                } else {
                    boneIndex = boneMapping[boneName];
                    boneOffsets[boneIndex] = glm::transpose(glm::make_mat4(&mesh->mBones[b]->mOffsetMatrix.a1));
                }

                for (unsigned int w = 0; w < mesh->mBones[b]->mNumWeights; ++w) {
                    unsigned int vertexID = vertexOffset + mesh->mBones[b]->mWeights[w].mVertexId;
                    float weight = mesh->mBones[b]->mWeights[w].mWeight;

                    Vertex& v = vertices[vertexID];
                    int& assigned = bonesAssigned[vertexID];

                    if (assigned < MAX_BONES_PER_VERTEX) {
                        v.boneIDs[assigned] = boneIndex;
                        v.weights[assigned] = weight;
                        ++assigned;
                    } else {
                        std::cerr << "Vertex " << vertexID << " has too many bone weights.\n";
                    }
                }
            }
        } else {
            const aiNode* host = findNodeWithMesh(scene->mRootNode, m);
            const std::string hostName = host ? host->mName.C_Str()
                                              : ("__rigid_mesh_" + std::to_string(m));

            int boneIndex;
            if (boneMapping.find(hostName) == boneMapping.end()) {
                boneIndex = numBones++;
                boneMapping[hostName] = boneIndex;
                boneOffsets.push_back(glm::mat4(1.0f));
                boneFinalTransforms.push_back(glm::mat4(1.0f));
            } else {
                boneIndex = boneMapping[hostName];
            }

            for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
                unsigned int vid = vertexOffset + i;
                vertices[vid].boneIDs[0] = boneIndex;
                vertices[vid].weights[0] = 1.0f;
                bonesAssigned[vid]       = 1;
            }
        }

        vertexOffset += mesh->mNumVertices;
    }

    boundingRadiusXZ = 0.0f;
    for (unsigned int m = 0; m < scene->mNumMeshes; ++m) {
        const aiMesh* mesh = scene->mMeshes[m];
        glm::mat4 toModelSpace(1.0f);
        if (!mesh->HasBones()) {
            if (const aiNode* host = findNodeWithMesh(scene->mRootNode, m))
                toModelSpace = nodeWorldTransform(host);
        }
        for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
            glm::vec4 local(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, 1.0f);
            glm::vec3 modelPos = glm::vec3(toModelSpace * local);
            float r = std::sqrt(modelPos.x * modelPos.x + modelPos.z * modelPos.z);
            if (r > boundingRadiusXZ) boundingRadiusXZ = r;
        }
    }

    for (size_t i = 0; i < vertices.size(); ++i) {
        float totalWeight = vertices[i].weights[0] + vertices[i].weights[1] +
                            vertices[i].weights[2] + vertices[i].weights[3];
        if (totalWeight > 1.0f) {
            for (int j = 0; j < MAX_BONES_PER_VERTEX; ++j)
                vertices[i].weights[j] /= totalWeight;
        } else if (totalWeight <= 0.0f) {
            vertices[i].boneIDs[0] = 0;
            vertices[i].weights[0] = 1.0f;
        }
    }

    if (scene->mNumAnimations > 0) {
        animationIndex = 0;
        unsigned int bestChannels = scene->mAnimations[0]->mNumChannels;
        double       bestDuration = scene->mAnimations[0]->mDuration;
        for (unsigned int a = 1; a < scene->mNumAnimations; ++a) {
            const aiAnimation* anim = scene->mAnimations[a];
            const bool better = (anim->mNumChannels > bestChannels) ||
                                (anim->mNumChannels == bestChannels && anim->mDuration > bestDuration);
            if (better) {
                animationIndex = a;
                bestChannels   = anim->mNumChannels;
                bestDuration   = anim->mDuration;
            }
        }

        runAnimationIndex = animationIndex;
        for (unsigned int a = 0; a < scene->mNumAnimations; ++a) {
            std::string name = scene->mAnimations[a]->mName.C_Str();
            std::transform(name.begin(), name.end(), name.begin(),
                [](unsigned char c) { return std::tolower(c); });
            if (name.find("run") != std::string::npos) {
                runAnimationIndex = a;
                break;
            }
        }

        const aiAnimation* chosen = scene->mAnimations[animationIndex];
        animationTicksPerSecond = static_cast<float>(chosen->mTicksPerSecond);
        if (animationTicksPerSecond <= 0.0f) animationTicksPerSecond = 25.0f;
        animationDuration    = static_cast<float>(chosen->mDuration);
        runAnimationDuration = static_cast<float>(scene->mAnimations[runAnimationIndex]->mDuration);
    }

    offsetToFeet = -maxY;
    numTriangles = indices.size() / 3;
    loadVAO();
}

/**
 * @brief Creates a simple triangle cross for GUI aiming purposes.
 */
void MeshGeometry::createCross() {
    float triangleVertices[] = {
        0.0f,  0.02f,
       -0.015f, -0.015f,
        0.015f, -0.015f
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    glBindVertexArray(0);
}

/**
 * @brief Generates a basic sphere mesh (used for sky and light objects).
 */
void MeshGeometry::generateSimpleSphereMesh() {
    for (int lat = 0; lat <= SPHERE_LAT; ++lat) {
        float theta = lat * glm::pi<float>() / SPHERE_LAT;
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);

        for (int lon = 0; lon <= SPHERE_LON; ++lon) {
            float phi = lon * 2.0f * glm::pi<float>() / SPHERE_LON;
            float sinPhi = sin(phi);
            float cosPhi = cos(phi);

            Vertex vertex;
            vertex.position = glm::vec3(cosPhi * sinTheta, cosTheta, sinPhi * sinTheta);
            vertex.normal = vertex.position;
            vertex.texCoord = glm::vec2(1.0f - (float)lon / SPHERE_LON, 1.0f - (float)lat / SPHERE_LAT);

            vertices.push_back(vertex);
        }
    }

    for (int lat = 0; lat < SPHERE_LAT; ++lat) {
        for (int lon = 0; lon < SPHERE_LON; ++lon) {
            int first = lat * (SPHERE_LON + 1) + lon;
            int second = first + SPHERE_LON + 1;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }

    numTriangles = indices.size() / 3;
    numVertices = vertices.size();
    loadVAO();
}

/**
 * @brief Uploads vertex and index data to GPU and configures attribute pointers.
 */
void MeshGeometry::loadVAO() {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    GLsizei stride = sizeof(Vertex);
    glEnableVertexAttribArray(0); // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, position));

    glEnableVertexAttribArray(2); // normal
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, normal));

    glEnableVertexAttribArray(1); // texCoord
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, texCoord));

    glEnableVertexAttribArray(3); // color
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, color));

    glEnableVertexAttribArray(4); // bone IDs
    glVertexAttribIPointer(4, 4, GL_INT, stride, (void*)offsetof(Vertex, boneIDs));

    glEnableVertexAttribArray(5); // weights
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, weights));

    glBindVertexArray(0);
}

/**
 * @brief Cleans up GPU resources allocated for the mesh.
 */
void MeshGeometry::cleanupGeometry() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &vbo);
}