/**
 * @file gamestate.h
 * @brief Defines the GameState structure to manage runtime state of the application.
 */

#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "lib.h"

#include "data.h"

 /**
  * @struct GameState
  * @brief Maintains global application and gameplay state.
  */
struct GameState {

    /** @brief Current window width (updated on reshape). */
    int windowWidth;

    /** @brief Current window height (updated on reshape). */
    int windowHeight;

    /** @brief Flag indicating if the game is over. */
    bool gameOver;

    /** @brief Map of currently pressed keys. */
    bool keyMap[KEYS_COUNT];

    /** @brief Whether the cursor is currently visible. */
    bool cursor = false;

    /** @brief Elapsed time in seconds since game start. */
    float elapsedTime;

    /** @brief Control points for the spline camera animation. */
    glm::vec3 splineControlPoints[4];

    /** @brief Current time of day [0, 1]. */
    float timeOfDay = 0.0f;

    /** @brief Flag to enable or disable day-night time advancement. */
    bool advanceTime = false;

    /** @brief Indicates whether it's currently night time. */
    bool isNight = false;

    /** @brief Whether the flashlight is turned on. */
    bool flashlightOn = false;

    /** @brief Whether the start menu should be shown. */
    bool showStartMenu = true;

    /** @brief Flag indicating if the player is using the boy model. */
    bool isBoy = true;

    /** @brief Start position of the stone object animation. */
    glm::vec3 stoneStart;

    /** @brief End position of the stone object animation. */
    glm::vec3 stoneEnd;

    /** @brief Control point for the stone's animation curve. */
    glm::vec3 stoneControl;

    /** @brief Last recorded camera position. */
    glm::vec3 lastCamPosition;

    /** @brief Last recorded camera direction. */
    glm::vec3 lastCamDirection;

    /** @brief Index of the current active camera mode. */
    int cameraIndex = 0;

    /** @brief Time along the current camera path animation. */
    float cameraPathTime = 0.0f;

    /** @brief True if camera is transitioning between views. */
    bool isCameraTransitioning = false;

    /** @brief Elapsed time since the camera transition started. */
    float cameraTransitionTime = 0.0f;

    /** @brief Start position for camera transition. */
    glm::vec3 cameraStartPos;

    /** @brief Start direction for camera transition. */
    glm::vec3 cameraStartDir;

    /** @brief Start look-at target for camera transition. */
    glm::vec3 cameraStartLookAt;

    /** @brief Target look-at point for camera transition. */
    glm::vec3 cameraTargetLookAt;

    /** @brief Target camera position for transition. */
    glm::vec3 cameraTargetPos;

    /** @brief Target camera direction for transition. */
    glm::vec3 cameraTargetDir;

    /** @brief Target camera mode index after transition. */
    int cameraTargetIndex = 0;

    /** @brief Elapsed time for continuous spline camera animation. */
    float splineAnimTime = 0.0f;

    /** @brief Current normalized spline interpolation parameter [0, 1]. */
    float currentSplineT = 0.0f;
};

#endif // GAMESTATE_H
