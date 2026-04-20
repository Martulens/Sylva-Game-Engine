/**
 * @file objects.h
 * @brief Declarations for all scene object classes including Object, Skybox, Light, Player, and InstanceGroup.
 */

#ifndef OBJECTS_H
#define OBJECTS_H

#include "sylva/sylva.h"
#include "meshgeometry.h"
#include "camera.h"
#include "modeltexture.h"
#include "shaders.h"
#include "data.h"
#include <unordered_map>

 /**
  * @class Object
  * @brief Base class for all renderable scene objects.
  */
class Object {
public:
    glm::vec3 position   = glm::vec3(0.0f);                  ///< Object world position.
    glm::vec3 direction  = glm::vec3(0.0f, 0.0f, -1.0f);     ///< Facing direction (optional use).
    float rotationY = 0.0f;       ///< Y-axis rotation.
    glm::mat4 modelMatrix = glm::mat4(1.0f); ///< Model transformation matrix.

    float speed = 0.1f;           ///< Optional speed for movement.
    float size = 1.0f;            ///< Optional size scalar.

    float starTime = 0.0f;        ///< Custom timer start.
    float currTime = 0.0f;        ///< Current time value.

    ShaderProgram* shader  = nullptr;   ///< Pointer to shader used for this object.
    MeshGeometry* geometry = nullptr;   ///< Pointer to mesh geometry.
    ModelTexture* texture  = nullptr;   ///< Pointer to object's texture(s).

    Object() = default;
    Object(glm::vec3 position);
    Object(glm::vec3 position, std::string vert, std::string frag, std::string model, std::string texture);
    Object(MeshGeometry* mesh, ShaderProgram* shader);

    /**
     * @brief Draws the object using its geometry and shader.
     */
    void draw();
};

/**
 * @class Skybox
 * @brief Represents a screen-space quad skybox with cube map texture.
 */
class Skybox : public Object {
public:
    Skybox() = default;
    Skybox(std::string vert, std::string frag);

    GLint screenCoordLocation;       ///< Location of screen-space coordinate attribute.
    GLint inversePVmatrixLocation;   ///< Inverse Projection-View matrix uniform.
    GLint skyboxSamplerLocation;     ///< Texture sampler uniform.

    /**
     * @brief Sets uniform locations.
     */
    void setUniforms();

    /**
     * @brief Draws the skybox.
     * @param viewMatrix View matrix with translation removed.
     * @param projectionMatrix Projection matrix.
     */
    void draw(glm::mat4 viewMatrix, glm::mat4 projectionMatrix);
};

/**
 * @class InstanceGroup
 * @brief Handles rendering of multiple instanced meshes (e.g. trees, torches).
 */
class InstanceGroup : public Object {
public:
    std::vector<std::vector<glm::mat4>> instanceMatrices;   ///< Per-mesh model matrices.
    std::vector<MeshGeometry*> meshes;                      ///< Mesh list for instancing.
    std::vector<int> countInstances;                        ///< Instance count per mesh.
    std::vector<GLuint> instancesVBO;                       ///< Instance buffer objects.

    InstanceGroup() = default;
    InstanceGroup(std::vector<glm::mat4>& mMatrices, ShaderProgram* s, ModelTexture* tex, int i);

    /**
     * @brief Adds instances to a specific mesh index.
     * @param newMatrices Map from mesh index to model matrix list.
     */
    void addInstances(std::map<int, std::vector<glm::mat4>>& newMatrices);

    /**
     * @brief Updates VBO with new matrix data.
     * @param meshIndex Index of mesh in instance group.
     */
    void updateInstanceBuffer(int meshIndex);

    /**
     * @brief Draws all instances in this group.
     */
    void draw();

    /**
     * @brief Loads meshes based on category index.
     * @param i Category index (e.g. tree, detail, torch).
     */
    void loadMeshes(int i);
};

/**
 * @class Light
 * @brief Represents directional or point light (sun/moon).
 */
class Light : public Object {
public:
    bool isMoon = false;                ///< Is this light the moon (affects lighting logic)?
    glm::vec3 color;                    ///< RGB color.

    float fogDensity = FOG_DENSITY;     ///< Scene fog density from this light.
    float fogGradient = FOG_GRADIENT;   ///< Scene fog gradient.
    glm::vec3 skyColor = skyColorConst; ///< Sky color at horizon.

    float brightness = 0.0f;            ///< Light intensity.

    Light() = default;
    Light(MeshGeometry* mesh, ShaderProgram* shader, ModelTexture* tex, glm::vec3 color);
};

/**
 * @class Player
 * @brief Main controllable player with animation state and movement.
 */
class Player : public Object {
public:
    Player(glm::vec3 pos, glm::vec3 dir, MeshGeometry* mesh, ModelTexture* texture, ShaderProgram* shader);

    PlayerState state = IDLE;              ///< Current player state.

    glm::vec3 renderFacingDirection = glm::vec3(0.0f, 0.0f, -1.0f); ///< Direction for visual rotation.
    float viewAngle = 0.0f;                ///< Yaw.
    float pitch = 0.0f;                    ///< Pitch.

    float animationTime = 0.0f;            ///< Current time in animation.
    float tickPerSecond = 25.0f;           ///< Animation speed.

    float runStart = 2.0f;                 ///< Start tick of running animation.
    float runEnd = 45.0f;                  ///< End tick of running animation.

    bool idle = true;                      ///< Flag if idle animation should play.
    unsigned int currentAnimIndex = 0;     ///< Current animation index.

    /**
     * @brief Draws the animated player.
     */
    void draw();

    /**
     * @brief Moves player forward.
     * @param deltaSpeed Delta time or movement scale.
     */
    void MoveFront(float deltaSpeed);

    /**
     * @brief Moves player backward.
     * @param deltaSpeed Delta time or movement scale.
     */
    void MoveBack(float deltaSpeed);

    /**
     * @brief Strafes player left.
     * @param deltaAngle Delta time or speed.
     * @param up World up direction.
     */
    void MoveLeft(float deltaAngle, glm::vec3& up);

    /**
     * @brief Strafes player right.
     * @param deltaAngle Delta time or speed.
     * @param up World up direction.
     */
    void MoveRight(float deltaAngle, glm::vec3& up);
};

#endif // OBJECTS_H
