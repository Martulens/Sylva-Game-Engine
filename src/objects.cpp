/**
 * @file objects.cpp
 * @brief Implementation of game objects: Object, Light, Skybox, InstanceGroup, Player.
 */

#include <math.h>
#include <algorithm>
#include <time.h>
#include <iostream>

#include "data.h"
#include "objects.h"
#include "sylva/sylva.h"

/**
 * @brief Constructs a renderable object with full resource initialization.
 * @param pos Position of the object.
 * @param vert Vertex shader filename.
 * @param frag Fragment shader filename.
 * @param model Model file path.
 * @param tex Texture file path.
 */
Object::Object(glm::vec3 pos, std::string vert, std::string frag, std::string model, std::string tex) {
    position = pos;
    geometry = new MeshGeometry(model);
    texture = new ModelTexture(tex);
    shader = new ShaderProgram(vert, frag);
}

/**
 * @brief Constructs a minimal object with just position.
 * @param pos Position of the object.
 */
Object::Object(glm::vec3 pos) {
    position = pos;
}

/**
 * @brief Constructs an object with shared geometry and shader.
 * @param mesh Pointer to mesh geometry.
 * @param s Pointer to shader program.
 */
Object::Object(MeshGeometry* mesh, ShaderProgram* s) {
    geometry = mesh;
    shader = s;
}

/**
 * @brief Constructs a light object.
 * @param mesh Pointer to the mesh geometry.
 * @param s Pointer to shader program.
 * @param tex Pointer to light texture.
 * @param col Light color vector.
 */
Light::Light(MeshGeometry* mesh, ShaderProgram* s, ModelTexture* tex, glm::vec3 col) {
    geometry = mesh;
    shader = s;
    texture = tex;
    color = col;
}

/**
 * @brief Constructs and initializes a cubemap skybox.
 * @param vert Vertex shader filename.
 * @param frag Fragment shader filename.
 */
Skybox::Skybox(std::string vert, std::string frag) {
    shader = new ShaderProgram(vert, frag);
    geometry = new MeshGeometry();

    static const float screenCoords[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f,  1.0f
    };

    glGenVertexArrays(1, &geometry->vao);
    glBindVertexArray(geometry->vao);

    glGenBuffers(1, &geometry->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, geometry->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(screenCoords), screenCoords, GL_STATIC_DRAW);

    glEnableVertexAttribArray(screenCoordLocation);
    glVertexAttribPointer(screenCoordLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindVertexArray(0);
    glUseProgram(0);
    CHECK_GL_ERROR();

    geometry->numTriangles = 2;

    glActiveTexture(GL_TEXTURE0);

    texture = new ModelTexture();
    glGenTextures(1, &texture->textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture->textureID);

    std::string faces[6] = {
        "textures/skybox/right.png",
        "textures/skybox/left.png",
        "textures/skybox/top.png",
        "textures/skybox/bottom.png",
        "textures/skybox/front.png",
        "textures/skybox/back.png"
    };

    GLuint targets[6] = {
        GL_TEXTURE_CUBE_MAP_POSITIVE_X,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
    };

    for (int i = 0; i < 6; ++i) {
        std::cout << "[Skybox] Loading face: " << faces[i] << std::endl;
        if (!sylva::loadTexImage2D(faces[i], targets[i])) {
            std::cerr << "[Skybox] Failed to load: " << faces[i] << std::endl;
            sylva::dieWithError("Skybox texture loading failed");
        }
    }

    CHECK_GL_ERROR();

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    glUseProgram(shader->program);
    GLint loc = glGetUniformLocation(shader->program, "skybox");
    glUniform1i(loc, 0);
    glUseProgram(0);

    setUniforms();
}

/**
 * @brief Caches uniform/attribute locations for skybox rendering.
 */
void Skybox::setUniforms() {
    inversePVmatrixLocation = glGetUniformLocation(shader->program, "inversePVMmatrix");
    screenCoordLocation = glGetAttribLocation(shader->program, "screenCoord");
    skyboxSamplerLocation = glGetUniformLocation(shader->program, "skyboxSampler");
}

/**
 * @brief Draws the skybox using current view and projection matrices.
 * @param viewMatrix View matrix (camera orientation).
 * @param projectionMatrix Projection matrix.
 */
void Skybox::draw(const glm::mat4 viewMatrix, const glm::mat4 projectionMatrix) {
    glDepthFunc(GL_LEQUAL);
    glUseProgram(shader->program);

    glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(viewMatrix));
    glUniformMatrix4fv(shader->viewMatrix, 1, GL_FALSE, glm::value_ptr(viewNoTranslation));
    glUniformMatrix4fv(shader->projectionMatrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    glBindVertexArray(geometry->vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture->textureID);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    glUseProgram(0);
    glDepthFunc(GL_LESS);
}

/**
 * @brief Loads meshes for instance rendering based on type index.
 * @param i Instance type (0: trees, 1: details, 2: torches).
 */
void InstanceGroup::loadMeshes(int i) {
    if (i == 1) {
        for (auto name : modelsFiles) {
            MeshGeometry* mesh = new MeshGeometry(name);
            meshes.push_back(mesh);
        }
    } else if (i == 2) {
        meshes.push_back(new MeshGeometry("models/torch.fbx"));
    } else {
        for (auto name : treesFiles) {
            MeshGeometry* mesh = new MeshGeometry(name);
            meshes.push_back(mesh);
        }
    }
}

/**
 * @brief Constructs an instance group with shader, texture and mesh initialization.
 * @param mMatrices Initial model matrices.
 * @param s Shader program pointer.
 * @param tex Texture pointer.
 * @param i Instance type index.
 */
InstanceGroup::InstanceGroup(std::vector<glm::mat4>& mMatrices, ShaderProgram* s, ModelTexture* tex, int i) {
    shader = s;
    texture = tex;
    loadMeshes(i);

    countInstances = std::vector<int>(meshes.size(), 0);
    instanceMatrices = std::vector<std::vector<glm::mat4>>(meshes.size());

    for (size_t j = 0; j < meshes.size(); j++) {
        GLuint vboIndex;
        glGenBuffers(1, &vboIndex);
        glBindVertexArray(meshes[j]->vao);
        instancesVBO.push_back(vboIndex);
    }
}

/**
 * @brief Updates the instance buffer for a specific mesh index.
 * @param meshIndex Index of the mesh in the instance group.
 */
void InstanceGroup::updateInstanceBuffer(int meshIndex) {
    glBindBuffer(GL_ARRAY_BUFFER, instancesVBO[meshIndex]);
    glBufferData(GL_ARRAY_BUFFER,
                 instanceMatrices[meshIndex].size() * sizeof(glm::mat4),
                 instanceMatrices[meshIndex].data(), GL_DYNAMIC_DRAW);
}

/**
 * @brief Adds instances to the group and updates their GPU buffers.
 * @param newMatrices Map of mesh index to list of transformation matrices.
 */
void InstanceGroup::addInstances(std::map<int, std::vector<glm::mat4>>& newMatrices) {
    for (auto& pair: newMatrices) {
        auto index = pair.first;
        auto matrices = pair.second;
        if (matrices.empty()) continue;

        instanceMatrices[index].insert(instanceMatrices[index].end(), matrices.begin(), matrices.end());
        countInstances[index] = instanceMatrices[index].size();

        glBindVertexArray(meshes[index]->vao);
        glBindBuffer(GL_ARRAY_BUFFER, instancesVBO[index]);
        glBufferData(GL_ARRAY_BUFFER, instanceMatrices[index].size() * sizeof(glm::mat4),
                     instanceMatrices[index].data(), GL_DYNAMIC_DRAW);

        std::size_t vec4Size = sizeof(glm::vec4);
        for (unsigned int i = 0; i < 4; i++) {
            glEnableVertexAttribArray(4 + i);
            glVertexAttribPointer(4 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(i * vec4Size));
            glVertexAttribDivisor(4 + i, 1);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

/**
 * @brief Draws the object using its geometry and shader.
 */
void Object::draw() {
    if (geometry != nullptr) {
        glUseProgram(shader->program);
        glBindVertexArray(geometry->vao);
        glDrawElements(GL_TRIANGLES, geometry->numTriangles * 3, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        glUseProgram(0);
    }
}

/**
 * @brief Constructs a player character object.
 * @param pos Starting position.
 * @param dir Starting direction.
 * @param mesh Mesh geometry.
 * @param tex Texture.
 * @param s Shader program.
 */
Player::Player(glm::vec3 pos, glm::vec3 dir, MeshGeometry* mesh, ModelTexture* tex, ShaderProgram* s) {
    position = pos;
    direction = glm::normalize(dir);
    renderFacingDirection = direction;
    geometry = mesh;
    texture = tex;
    shader = s;
}

/**
 * @brief Draws the player model.
 */
void Player::draw() {
    glBindVertexArray(geometry->vao);
    glDrawElements(GL_TRIANGLES, geometry->numTriangles * 3, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

/**
 * @brief Moves the player forward.
 * @param deltaTime Time elapsed for frame.
 */
void Player::MoveFront(float deltaTime) {
    position += PLAYER_BASE_SPEED * deltaTime * direction;
}

/**
 * @brief Moves the player backward.
 * @param deltaTime Time elapsed for frame.
 */
void Player::MoveBack(float deltaTime) {
    position -= PLAYER_BASE_SPEED * deltaTime * direction;
}

/**
 * @brief Moves the player left relative to direction and up vector.
 * @param deltaTime Time elapsed.
 * @param up Camera up vector.
 */
void Player::MoveLeft(float deltaTime, glm::vec3& up) {
    glm::vec3 right = glm::normalize(glm::cross(direction, up));
    position -= PLAYER_BASE_SPEED * deltaTime * right;
}

/**
 * @brief Moves the player right relative to direction and up vector.
 * @param deltaTime Time elapsed.
 * @param up Camera up vector.
 */
void Player::MoveRight(float deltaTime, glm::vec3& up) {
    glm::vec3 right = glm::normalize(glm::cross(direction, up));
    position += PLAYER_BASE_SPEED * deltaTime * right;
}
