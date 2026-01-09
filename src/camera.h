/**
 * @file camera.h
 * @brief Declaration of the Camera class for managing view and projection matrices.
 */

#ifndef CAMERA_CLASS_H
#define CAMERA_CLASS_H

#include "objects.h"
#include "glm/glm.hpp"
#include "lib.h"

class Player;

/**
 * @class Camera
 * @brief Manages view and projection matrices for both first-person and third-person perspectives.
 */
class Camera {
public:
    glm::vec3 position;              ///< Camera position in world space
    glm::vec3 direction;             ///< Direction camera is facing
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f); ///< Up vector

    glm::mat4 viewMatrix;           ///< View matrix
    glm::mat4 projectionMatrix;     ///< Projection matrix

    int width;                      ///< Viewport width
    int height;                     ///< Viewport height

    bool firstPerson = true;        ///< Flag to switch between first-person and third-person

    float pitch = 20.0f;            ///< Vertical angle in third-person view
    float angleAroundPlayer = 0.0f; ///< Horizontal angle around the player
    float distanceFromPlayer = 30.0f; ///< Distance from the player in third-person view

    float fieldOfView = 60.0f;      ///< Field of view for perspective projection

    /**
     * @brief Default constructor.
     */
    Camera() = default;

    /**
     * @brief Construct a new Camera object.
     * @param w Viewport width.
     * @param h Viewport height.
     * @param pos Initial camera position.
     * @param up Up vector.
     * @param angle Initial view angle.
     */
    Camera(int w, int h, glm::vec3 pos, glm::vec3 up, float angle);

    /**
     * @brief Updates camera based on player position and screen size.
     * @param player Pointer to the player object.
     * @param w Optional new width (default -1).
     * @param h Optional new height (default -1).
     */
    void update(Player* player, int w = -1, int h = -1);

    /**
     * @brief Recalculates the projection matrix.
     */
    void updateProjection();

    /**
     * @brief Zooms in/out in third-person mode.
     * @param offset Zoom offset.
     */
    void zoom(float offset);

    /**
     * @brief Rotates the pitch (vertical angle) in third-person mode.
     * @param offset Pitch rotation offset.
     */
    void rotatePitch(float offset);

    /**
     * @brief Rotates the angle around the player in third-person mode.
     * @param offset Horizontal rotation offset.
     */
    void rotateAngleAround(float offset);

    /**
     * @brief Computes a world space direction vector from mouse screen coordinates.
     * @param mouseX Mouse X position.
     * @param mouseY Mouse Y position.
     * @param screenWidth Width of the screen.
     * @param screenHeight Height of the screen.
     * @return Normalized direction vector.
     */
    glm::vec3 getMouseRay(int mouseX, int mouseY, int screenWidth, int screenHeight);

    /**
     * @brief Tests for ray-AABB intersection.
     * @param rayOrigin Ray origin.
     * @param rayDir Ray direction.
     * @param boxMin Minimum coordinates of the AABB.
     * @param boxMax Maximum coordinates of the AABB.
     * @return True if intersecting, false otherwise.
     */
    bool rayIntersectsAABB(glm::vec3 rayOrigin, glm::vec3 rayDir, glm::vec3 boxMin, glm::vec3 boxMax);

private:
    /**
     * @brief Updates the camera in first-person mode.
     * @param player Pointer to the player.
     */
    void updateFirstPerson(Player* player);

    /**
     * @brief Updates the camera in third-person mode.
     * @param player Pointer to the player.
     */
    void updateThirdPerson(Player* player);
};

/**
 * @brief Computes a Catmull-Rom spline interpolation between four points.
 * @param p0 First control point.
 * @param p1 Second control point.
 * @param p2 Third control point.
 * @param p3 Fourth control point.
 * @param t Interpolation parameter [0,1].
 * @return Interpolated point on the spline.
 */
glm::vec3 catmullRomSpline(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float t);

#endif CAMERA_CLASS_H
