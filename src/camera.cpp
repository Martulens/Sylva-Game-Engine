/**
 * @file camera.cpp
 * @brief Implementation of the Camera class for handling first-person and third-person camera views.
 */

#include "camera.h"
#include "sylva/sylva.h"
#include <iostream>

 /**
  * @brief Constructs a Camera object and initializes the projection matrix.
  * @param windowWidth Width of the window in pixels.
  * @param windowHeight Height of the window in pixels.
  * @param initialPosition Initial camera position.
  * @param viewDirection Initial viewing direction.
  * @param viewAngle Initial camera angle.
  */
Camera::Camera(int windowWidth, int windowHeight, glm::vec3 initialPosition, glm::vec3 viewDirection, float viewAngle) {
    width = windowWidth;
    height = windowHeight;
    direction = glm::normalize(viewDirection);
    updateProjection();
}

/**
 * @brief Updates the camera based on the current mode (first or third person).
 * @param player Pointer to the player object for tracking.
 * @param windowWidth Optional new window width.
 * @param windowHeight Optional new window height.
 */
void Camera::update(Player* player, int windowWidth, int windowHeight) {
    if (windowWidth > 0) width = windowWidth;
    if (windowHeight > 0) height = windowHeight;

    if (firstPerson)
        updateFirstPerson(player);
    else
        updateThirdPerson(player);

    updateProjection();
}

/**
 * @brief Recalculates the projection matrix using the current aspect ratio.
 */
void Camera::updateProjection() {
    float aspect = (height > 0) ? static_cast<float>(width) / height : 1.0f;
    projectionMatrix = glm::perspective(glm::radians(fieldOfView), aspect, 0.1f, 1000.0f);
}

/**
 * @brief Updates the camera position and view matrix in first-person mode.
 * @param player Pointer to the player whose perspective is followed.
 */
void Camera::updateFirstPerson(Player* player) {
    if (player) {
        position = player->position + glm::vec3(1.75f * player->direction.x, 4.5f, 1.75f * player->direction.z);
        direction = glm::normalize(player->direction);
        viewMatrix = glm::lookAt(position, position + direction, up);
    }
}

/**
 * @brief Updates the camera position and view matrix in third-person mode.
 * @param player Pointer to the player to orbit around.
 */
void Camera::updateThirdPerson(Player* player) {
    float horizontalDistance = distanceFromPlayer * cos(glm::radians(pitch));
    float verticalDistance = distanceFromPlayer * sin(glm::radians(pitch));

    float theta = glm::radians(angleAroundPlayer);
    float offsetX = horizontalDistance * sin(theta);
    float offsetZ = horizontalDistance * cos(theta);

    position = player->position - glm::vec3(offsetX, -verticalDistance, offsetZ);
    direction = glm::normalize(player->position - position);
    viewMatrix = glm::lookAt(position, player->position, up);
}

/**
 * @brief Adjusts the zoom level of the third-person camera.
 * @param offset Amount to zoom in or out.
 */
void Camera::zoom(float offset) {
    distanceFromPlayer = glm::clamp(distanceFromPlayer - offset, 5.0f, 100.0f);
}

/**
 * @brief Rotates the camera's pitch angle.
 * @param offset Change in pitch.
 */
void Camera::rotatePitch(float offset) {
    pitch = glm::clamp(pitch + offset, 5.0f, 85.0f);
}

/**
 * @brief Rotates the camera around the player horizontally.
 * @param offset Change in angle around the player.
 */
void Camera::rotateAngleAround(float offset) {
    angleAroundPlayer += offset;
}

/**
 * @brief Computes a ray from the camera through a screen point.
 * @param mouseX Mouse x-coordinate.
 * @param mouseY Mouse y-coordinate.
 * @param screenWidth Width of the screen.
 * @param screenHeight Height of the screen.
 * @return Normalized 3D direction of the ray.
 */
glm::vec3 Camera::getMouseRay(int mouseX, int mouseY, int screenWidth, int screenHeight) {
    float x = (2.0f * mouseX) / screenWidth - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / screenHeight;
    glm::vec4 rayClip(x, y, -1.0f, 1.0f);

    glm::vec4 rayEye = glm::inverse(projectionMatrix) * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

    glm::vec4 rayWorld = glm::inverse(viewMatrix) * rayEye;
    return glm::normalize(glm::vec3(rayWorld));
}

/**
 * @brief Tests if a ray intersects an axis-aligned bounding box (AABB).
 * @param rayOrigin Origin of the ray.
 * @param rayDir Direction of the ray.
 * @param boxMin Minimum corner of the AABB.
 * @param boxMax Maximum corner of the AABB.
 * @return True if the ray intersects the AABB, false otherwise.
 */
bool Camera::rayIntersectsAABB(glm::vec3 rayOrigin, glm::vec3 rayDir, glm::vec3 boxMin, glm::vec3 boxMax) {
    float tMin = (boxMin.x - rayOrigin.x) / rayDir.x;
    float tMax = (boxMax.x - rayOrigin.x) / rayDir.x;
    if (tMin > tMax) std::swap(tMin, tMax);

    float tyMin = (boxMin.y - rayOrigin.y) / rayDir.y;
    float tyMax = (boxMax.y - rayOrigin.y) / rayDir.y;
    if (tyMin > tyMax) std::swap(tyMin, tyMax);

    if ((tMin > tyMax) || (tyMin > tMax)) return false;

    tMin = std::max(tMin, tyMin);
    tMax = std::min(tMax, tyMax);

    float tzMin = (boxMin.z - rayOrigin.z) / rayDir.z;
    float tzMax = (boxMax.z - rayOrigin.z) / rayDir.z;
    if (tzMin > tzMax) std::swap(tzMin, tzMax);

    return !((tMin > tzMax) || (tzMin > tMax));
}

/**
 * @brief Evaluates a point on a Catmull-Rom spline.
 * @param p0 First control point.
 * @param p1 Second control point.
 * @param p2 Third control point.
 * @param p3 Fourth control point.
 * @param t Normalized time (0.0 - 1.0).
 * @return Point on the spline.
 */
glm::vec3 catmullRomSpline(const glm::vec3& p0, const glm::vec3& p1,
    const glm::vec3& p2, const glm::vec3& p3, float t) {
    float t2 = t * t;
    float t3 = t2 * t;

    return 0.5f * ((2.0f * p1) +
        (-p0 + p2) * t +
        (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 +
        (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3);
}