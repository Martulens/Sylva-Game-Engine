/**
 * @file data.h
 * @brief Contains global constants, enums, and configuration values for the project.
 */

#ifndef DATA_H
#define DATA_H

#include "pgr.h"
#include <stdio.h>
#include <vector>

 /** @brief Default window width */
const int WINDOW_WIDTH = 1600;
/** @brief Default window height */
const int WINDOW_HEIGHT = 1000;
/** @brief Frame refresh time in milliseconds */
const int refreshTimeMs = 33;

/** @brief Constant sky color used as background */
const glm::vec3 skyColorConst = glm::vec3(0.5f, 0.4f, 0.8f);

/** @brief Latitude resolution of generated sphere */
const int SPHERE_LAT = 20;
/** @brief Longitude resolution of generated sphere */
const int SPHERE_LON = 20;

/** @brief List of model files used as detail objects */
const std::vector<std::string> modelsFiles = {
    "models/Flowers/red.fbx",
    "models/Logs/logRegular.fbx",
    "models/Bushes/bushA.fbx",
    "models/Branches/branchA.fbx",
    "models/Grass/GrassA.fbx",
    "models/Grass/GrassB.fbx",
    "models/Mushrooms/groupBrown.fbx",
    "models/Stones/stoneA.fbx",
    "models/Stones/stoneB.fbx"
};

/** @brief List of model files used for trees */
const std::vector<std::string> treesFiles = {
    "models/DTree/treeA.fbx",
    "models/DTree/treeB.fbx",
    "models/DTree/treeC.fbx",
    "models/DTree/treeD.fbx",
    "models/DTree/treeE.fbx",
    "models/CTree/treeA.fbx",
    "models/CTree/treeB.fbx",
    "models/CTree/treeC.fbx",
    "models/CTree/treeD.fbx"
};

/**
 * @enum KeyBindings
 * @brief Enum for keyboard key mapping.
 */
enum {
    KEY_LEFT_ARROW,
    KEY_RIGHT_ARROW,
    KEY_UP_ARROW,
    KEY_DOWN_ARROW,
    KEY_SPACE,
    KEYS_COUNT
};

/**
 * @enum MenuOptions
 * @brief Enum for menu selections.
 */
enum MenuOptions {
    MENU_START_GAME,
    MENU_EXIT
};

/**
 * @enum PlayerState
 * @brief Enum for animation states of the player.
 */
enum PlayerState {
    IDLE,
    WAVING,
    RUNNING
};

#define CAMERA_TRANSITION_DURATION 2.0f
#define SKYBOX_CUBE_TEXTURE_FILE_PREFIX "textures/skybox/"

#define MAX_BONES 100
#define MAX_BONES_PER_VERTEX 30

#define TREE_GENERATION_RADIUS 50
#define TREE_SPACING 25.0f
#define TREE_COUNT 200

#define DETAILS_GENERATION_RADIUS 15
#define DETAILS_SPACING 5.0f
#define DETAILS_COUNT 350

#define CHUNK_SIZE 300
#define CUTOFF_ANGLE 20.0f

#define PLAYER_BASE_SPEED       15.0f
#define PLAYER_SPEED_MAX        20.0f
#define PLAYER_VIEW_ANGLE_DELTA 3.0f
#define PLAYER_MAX_ANGLE        45.0f
#define PLAYER_FOOT_OFFSET     -1.0f

#define PLAYER_MOUSE_SENSITIVITY 0.05f

#define MAX_POINT_LIGHTS 20

#define TERRAIN_SIZE 800
#define VERTEX_COUNT_TERRAIN 128

#define FOG_DENSITY 0.003
#define FOG_GRADIENT 1.5

#endif // DATA_H
