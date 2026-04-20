/**
 * @file main.cpp
 * @brief Sylva application entry point — GLFW window, input, and main loop.
 */

#include <iostream>
#include <list>
#include <set>
#include <utility>
#include <algorithm>

#include "sylva/sylva.h"
#include <GLFW/glfw3.h>

#include "data.h"
#include "camera.h"
#include "render.h"
#include "gamestate.h"
#include "objects.h"
#include "terrain.h"

GameState gameState;
Camera mainCamera;

// Mouse state variables
int lastMouseX = 0;
int lastMouseY = 0;
int xOffset = 0;
int yOffset = 0;
bool leftMouseDown = false;
bool rightMouseDown = false;

static GLFWwindow* gWindow = nullptr;

static double gLastRawMouseX = 0.0;
static double gLastRawMouseY = 0.0;
static bool gHaveLastRawMouse = false;

// Trees and details data
ModelTexture* treemapTexture = nullptr;
std::vector<unsigned char> treemapPixels;

int treemapWidth = 0;
int treemapHeight = 0;

std::set<std::pair<int, int>> generatedTreeChunks;
std::set<std::pair<int, int>> generatedDetailsChunks;

// Remembering placed positions and their collision spheres
std::vector<glm::vec3> placedPositions;
std::vector<std::pair<glm::vec3, float>> collisionSpheres;

/**
 * @brief Stores all major in-game objects.
 */
struct GameObjects {
    Player* player;
    Terrain* terrain;

    InstanceGroup trees;
    InstanceGroup details;
    InstanceGroup torches;

    Light sunSphere;
    Light moonSphere;

    Skybox* skybox;
    Object cross;
} gameObjects;

void updatePlayerAndCamera();
void restartGame();
void addTorch();

glm::vec3 evaluateSunPosition(float t) {
    float angle = glm::radians(360.0f * (t - 0.4f));
    float radius = 500.0;
    return glm::vec3(radius * cos(angle), radius * sin(angle), 0.0f);
}

glm::vec3 evaluateMoonPosition(float t) {
    return evaluateSunPosition(fmod(t + 0.5f, 1.0f)) + glm::vec3(0, 20.0f, 0);
}

void updateMoonSun() {
    if (gameState.advanceTime) {
        gameState.timeOfDay += 0.0001f;

        if (gameState.timeOfDay > 1.0f)
            gameState.timeOfDay -= 1.0f;
    }

    glm::vec3 sunPos = evaluateSunPosition(gameState.timeOfDay);
    glm::vec3 moonPos = evaluateMoonPosition(gameState.timeOfDay);

    gameObjects.sunSphere.position = sunPos;
    gameObjects.moonSphere.position = moonPos;

    gameObjects.moonSphere.direction = glm::vec3(0.0f);
    gameObjects.sunSphere.direction = -glm::normalize(sunPos);

    float sunHeightFactor = glm::clamp(gameObjects.sunSphere.position.y / 100.0f, 0.0f, 1.0f);
    float moonHeightFactor = glm::clamp(gameObjects.moonSphere.position.y / 100.0f, 0.0f, 1.0f);

    gameObjects.sunSphere.brightness = sunHeightFactor;
    gameObjects.moonSphere.brightness = moonHeightFactor;

    gameState.isNight = sunPos.y < 0;

    float sunFactor = glm::clamp(gameObjects.sunSphere.position.y / 100.0f, 0.0f, 1.0f);

    if (gameState.isNight) {
        gameObjects.moonSphere.skyColor = glm::mix(glm::vec3(0.02f, 0.02f, 0.08f),
                                                   glm::vec3(0.5f, 0.7f, 1.0f),
                                                   sunFactor);
        gameObjects.moonSphere.fogDensity = glm::mix(0.005f, 0.0001f, sunFactor);
        gameObjects.moonSphere.fogGradient = glm::mix(2.5f, 1.2f, sunFactor);
        gameObjects.moonSphere.brightness = 1.0f - sunFactor;
    }
    else {
        gameObjects.sunSphere.skyColor = glm::mix(glm::vec3(0.02f, 0.02f, 0.08f),
                                                  glm::vec3(0.5f, 0.7f, 1.0f),
                                                  sunFactor);
        gameObjects.sunSphere.fogDensity = glm::mix(0.005f, 0.0001f, sunFactor);
        gameObjects.sunSphere.fogGradient = glm::mix(2.5f, 1.2f, sunFactor);
        gameObjects.sunSphere.brightness = sunFactor;
    }
}

void drawWindow() {
    int w = gameState.windowWidth;
    int h = gameState.windowHeight;

    if (h == 0)
        h = 1;

    glm::vec3 camPosition;
    if (gameState.cameraIndex == 0) {
        camPosition = gameObjects.player->position;
    }
    else
        camPosition = mainCamera.position;

    glm::vec3 camDirection = gameObjects.player->direction;
    float angle = gameObjects.player->viewAngle;

    if (gameState.isCameraTransitioning) {
        gameState.cameraTransitionTime += 0.016f;

        float tLinear = glm::clamp(gameState.cameraTransitionTime / 3.0f, 0.0f, 1.0f);
        float t = tLinear * tLinear * (3 - 2 * tLinear);

        glm::vec3 camPos = glm::mix(gameState.cameraStartPos, gameState.cameraTargetPos, t);
        glm::vec3 lookAt = glm::mix(gameState.cameraStartLookAt, gameState.cameraTargetLookAt, t);

        mainCamera.position = camPos;
        mainCamera.direction = glm::normalize(lookAt - camPos);

        mainCamera.viewMatrix = glm::lookAt(camPos, lookAt, glm::vec3(0, 1, 0));

        mainCamera.updateProjection();

        if (t >= 1.0f) {
            gameState.isCameraTransitioning = false;
            gameState.cameraIndex = gameState.cameraTargetIndex;
            mainCamera.firstPerson = (gameState.cameraTargetIndex == 0);

            mainCamera.position = gameState.cameraTargetPos;

            glm::vec3 dirVec = gameState.cameraTargetLookAt - gameState.cameraTargetPos;
            if (glm::length(dirVec) < 0.001f)
                dirVec = glm::vec3(0.0f, 0.0f, -1.0f);

            mainCamera.direction = glm::normalize(dirVec);

            if (!mainCamera.firstPerson) {
                mainCamera.pitch = 20.0f;
                mainCamera.angleAroundPlayer = 0.0f;
                mainCamera.distanceFromPlayer = 30.0f;
            }
        }
    }
    else {
        if (gameState.cameraIndex == 2 || gameState.cameraIndex == 3) {
            float speed = 0.05f;

            if (gameState.cameraIndex == 3) {
                gameState.splineAnimTime += 0.016f;
            }

            gameState.currentSplineT = fmod(gameState.splineAnimTime * speed, 1.0f);
            float t = gameState.currentSplineT;

            glm::vec3 center = gameObjects.player->position;

            std::vector<glm::vec3> points = {
                center + glm::vec3(-20, 80, 0),
                center + glm::vec3(0, 90, 20),
                center + glm::vec3(20, 80, 0),
                center + glm::vec3(0, 90, -20),
                center + glm::vec3(-20, 80, 0),
                center + glm::vec3(0, 90, 20),
                center + glm::vec3(20, 80, 0),
                center + glm::vec3(0, 90, -20)
            };

            int segmentCount = points.size() - 3;
            float splineT = t * segmentCount;
            int i = (int)splineT;
            float localT = splineT - i;

            i = glm::clamp(i, 0, segmentCount - 1);

            glm::vec3 p0 = points[i];
            glm::vec3 p1 = points[i + 1];
            glm::vec3 p2 = points[i + 2];
            glm::vec3 p3 = points[i + 3];

            glm::vec3 camPos = catmullRomSpline(p0, p1, p2, p3, localT);
            glm::vec3 lookAt = center;

            mainCamera.position = camPos;
            mainCamera.direction = glm::normalize(lookAt - camPos);
            mainCamera.viewMatrix = glm::lookAt(camPos, lookAt, glm::vec3(0, 1, 0));
            mainCamera.updateProjection();
        }
    }


    glm::mat4 projectionMatrix = mainCamera.projectionMatrix;
    glm::mat4 viewMatrix = mainCamera.viewMatrix;

    updateMoonSun();

    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);
    float sunFactor = glm::clamp(gameObjects.sunSphere.position.y / 100.0f, 0.0f, 1.0f);
    drawSkybox(gameObjects.skybox, mainCamera, sunFactor);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);

    Light& send = gameState.isNight ? gameObjects.moonSphere : gameObjects.sunSphere;

    drawSun(gameObjects.sunSphere, gameObjects.torches, mainCamera, viewMatrix, projectionMatrix, false, gameState,
            gameObjects.player);
    drawSun(gameObjects.moonSphere, gameObjects.torches, mainCamera, viewMatrix, projectionMatrix, true, gameState,
            gameObjects.player);
    CHECK_GL_ERROR();

    drawTerrain(gameObjects.terrain, gameObjects.torches, mainCamera, send, gameState, gameObjects.player);
    CHECK_GL_ERROR();

    drawTrees(gameObjects.trees, gameObjects.torches, mainCamera, send, gameState, gameObjects.player);
    drawTrees(gameObjects.details, gameObjects.torches, mainCamera, send, gameState, gameObjects.player);
    drawTrees(gameObjects.torches, gameObjects.torches, mainCamera, send, gameState, gameObjects.player);
    CHECK_GL_ERROR();

    drawPlayer(gameObjects.player, gameObjects.torches, mainCamera, send, gameState);
    CHECK_GL_ERROR();

    if (mainCamera.firstPerson)
        drawCross(gameObjects.cross);
    CHECK_GL_ERROR();

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glViewport(0, 0, gameState.windowWidth, gameState.windowHeight);

    CHECK_GL_ERROR();
}

static void framebufferSizeCallback(GLFWwindow*, int w, int h) {
    gameState.windowWidth = w;
    gameState.windowHeight = h;
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
}

void generateInstancesNear(float worldX, float worldZ, int type) {
    int chunkX = static_cast<int>(worldX) / CHUNK_SIZE;
    int chunkZ = static_cast<int>(worldZ) / CHUNK_SIZE;

    int count = (type == 0) ? TREE_COUNT : DETAILS_COUNT;
    float spacing = (type == 0) ? TREE_SPACING : DETAILS_SPACING;

    static std::set<std::pair<int, int>> generatedObjectChunks;
    if (generatedTreeChunks.count({chunkX, chunkZ}) > 0 && type == 0)
        return;
    if (generatedDetailsChunks.count({chunkX, chunkZ}) > 0 && type == 1)
        return;

    auto isBlack = [&](int x, int y) -> bool {
        if (x < 0 || y < 0 || x >= treemapWidth || y >= treemapHeight) return false;
        return treemapPixels[y * treemapWidth + x] < 50;
    };

    auto isFarEnough = [&](const glm::vec3& pos, const std::vector<glm::vec3>& existing, float minDist) {
        for (const auto& e : existing)
            if (glm::distance(e, pos) < minDist) return false;
        return true;
    };

    std::map<int, std::vector<glm::mat4>> newMatrices;

    float baseX = chunkX * CHUNK_SIZE;
    float baseZ = chunkZ * CHUNK_SIZE;

    const int maxTries = 1000;

    for (int i = 0, tries = 0; i < count && tries < maxTries; ++tries) {
        float x = baseX + (rand() % CHUNK_SIZE);
        float z = baseZ + (rand() % CHUNK_SIZE);

        if (x < 0 || x > TERRAIN_SIZE || z < 0 || z > TERRAIN_SIZE)
            continue;

        int imgX = static_cast<int>((x / TERRAIN_SIZE) * treemapWidth);
        int imgY = static_cast<int>((z / TERRAIN_SIZE) * treemapHeight);

        if (isBlack(imgX, imgY)) {
            float y = gameObjects.terrain->getHeight(x, z);
            glm::vec3 pos(x, y - 0.5f, z);

            const glm::vec3 playerPos = gameObjects.player
                                            ? gameObjects.player->position
                                            : mainCamera.position;
            if (glm::distance(pos, playerPos) < 5.0f)
                continue;

            if (isFarEnough(pos, placedPositions, spacing)) {
                float rotY = glm::radians(static_cast<float>(rand() % 360));

                glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), pos) *
                    glm::rotate(glm::mat4(1.0f), rotY, glm::vec3(0.0f, 1.0f, 0.0f)) *
                    glm::scale(glm::mat4(1.0f), glm::vec3(2.0f));

                int size = (type == 0) ? gameObjects.trees.meshes.size() : gameObjects.details.meshes.size();

                int index = rand() % size;
                newMatrices[index].push_back(modelMatrix);
                placedPositions.push_back(pos);
                collisionSpheres.emplace_back(pos, 1.5);
                ++i;
            }
        }
    }

    std::cout << "Generating trees in chunk: " << chunkX << ", " << chunkZ << std::endl;

    if (!newMatrices.empty()) {
        if (type == 0)
            gameObjects.trees.addInstances(newMatrices);
        else
            gameObjects.details.addInstances(newMatrices);
    }

    if (type == 0)
        generatedTreeChunks.insert({chunkX, chunkZ});
    if (type == 1)
        generatedDetailsChunks.insert({chunkX, chunkZ});
}

bool isColliding(glm::vec3 newPos) {
    float playerRadius = 0.5f;
    for (const auto& pair : collisionSpheres) {
        auto objPos = pair.first;
        auto radius = pair.second;

        if (glm::distance(newPos, objPos) < (radius + playerRadius)) {
            return true;
        }
    }
    return false;
}

void updatePlayer(float deltaTime) {
    bool isMoving = gameState.keyMap[KEY_UP_ARROW] || gameState.keyMap[KEY_DOWN_ARROW] ||
        gameState.keyMap[KEY_LEFT_ARROW] || gameState.keyMap[KEY_RIGHT_ARROW];

    if (isMoving) {
        gameObjects.player->state = RUNNING;
    } else if (gameObjects.player->state != WAVING) {
        gameObjects.player->state = IDLE;
    }

    MeshGeometry* geo = gameObjects.player->geometry;
    const float tps = geo->animationTicksPerSecond;

    float duration = 0.0f;
    switch (gameObjects.player->state) {
        case RUNNING: duration = geo->runAnimationDuration; break;
        case WAVING:  duration = geo->animationDuration;    break;
        case IDLE:
        default:      duration = 0.0f;                      break;
    }

    if (duration > 0.0f) {
        gameObjects.player->animationTime += deltaTime * tps;
        if (gameObjects.player->animationTime > duration)
            gameObjects.player->animationTime = fmod(gameObjects.player->animationTime, duration);
    }
    else {
        gameObjects.player->animationTime = 0.0f;
    }
}

void updatePlayerAndCamera() {
    float timeNow = sylva::elapsedSeconds();
    float deltaTime = timeNow - gameObjects.player->currTime;
    gameObjects.player->currTime = timeNow;
    glm::vec3 moveVector(0.0f);


    glm::vec3 prevPosition = gameObjects.player->position;
    if (gameState.keyMap[KEY_UP_ARROW]) {
        gameObjects.player->MoveFront(deltaTime);
        moveVector += gameObjects.player->direction;
    }
    if (gameState.keyMap[KEY_DOWN_ARROW]) {
        gameObjects.player->MoveBack(deltaTime);
        moveVector -= gameObjects.player->direction;
    }
    if (gameState.keyMap[KEY_LEFT_ARROW]) {
        gameObjects.player->MoveLeft(deltaTime, mainCamera.up);
        glm::vec3 right = glm::normalize(glm::cross(gameObjects.player->direction, mainCamera.up));
        moveVector -= right;
    }
    if (gameState.keyMap[KEY_RIGHT_ARROW]) {
        gameObjects.player->MoveRight(deltaTime, mainCamera.up);
        glm::vec3 right = glm::normalize(glm::cross(gameObjects.player->direction, mainCamera.up));
        moveVector += right;
    }

    if (glm::length(moveVector) > 0.01f) {
        float lerpFactor = 0.15f;
        gameObjects.player->renderFacingDirection = glm::normalize(
            glm::mix(gameObjects.player->renderFacingDirection, glm::normalize(moveVector), lerpFactor)
        );
    }

    if (isColliding(gameObjects.player->position)) {
        gameObjects.player->position = prevPosition;
    }

    if (gameObjects.player->position.x < 0.0f || gameObjects.player->position.x > TERRAIN_SIZE ||
        gameObjects.player->position.z < 0.0f || gameObjects.player->position.z > TERRAIN_SIZE) {
        gameObjects.player->position = prevPosition;
    }

    float height = gameObjects.terrain->getHeight(gameObjects.player->position.x,
                                                  gameObjects.player->position.z);
    gameObjects.player->position.y = height + 0.1;

    if (!gameState.isCameraTransitioning)
        mainCamera.update(gameObjects.player, gameState.windowWidth, gameState.windowHeight);

    generateInstancesNear(gameObjects.player->position.x, gameObjects.player->position.z, 0);
    generateInstancesNear(gameObjects.player->position.x, gameObjects.player->position.z, 1);

    xOffset = 0;
    yOffset = 0;

    updatePlayer(deltaTime);
}

void FPS() {
    static float lastTime = 0.0f;
    static int frameCount = 0;

    float currentTime = sylva::elapsedSeconds();
    frameCount++;

    if (currentTime - lastTime >= 1.0f) {
        std::cout << "FPS: " << frameCount << std::endl;
        frameCount = 0;
        lastTime += 1.0f;
    }
}

static void renderFrame() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    CHECK_GL_ERROR();
    drawWindow();
    updatePlayerAndCamera();

    CHECK_GL_ERROR();
    glfwSwapBuffers(gWindow);

    FPS();
}

void cleanUpObjects() {
    if (gameObjects.player)
        delete gameObjects.player;
    if (gameObjects.terrain)
        delete gameObjects.terrain;
    if (gameObjects.skybox)
        delete gameObjects.skybox;

    generatedTreeChunks.clear();
    generatedDetailsChunks.clear();
    placedPositions.clear();
    collisionSpheres.clear();
}

void initPlayer() {
    float x = 450.0f + rand() % 100;
    float z = 550.0f + rand() % 100;

    while (isColliding(glm::vec3(x, 0.0, z))) {
        x = 450.0f + rand() % 100;
        z = 550.0f + rand() % 100;
    }

    float height = gameObjects.terrain->getHeight(x, z);
    float y = height;

    std::string name = "models/boy/runA.fbx";
    MeshGeometry* mesh = new MeshGeometry(name);
    ModelTexture* texture = new ModelTexture("textures/characters.png");
    ShaderProgram* shader = new ShaderProgram("player.vert", "player.frag");

    gameObjects.player = new Player(glm::vec3(x, y, z), glm::normalize(glm::vec3(-1.0f, 0.0f, -1.0f)), mesh, texture,
                                    shader);
}

void initInstances(int i) {
    ModelTexture* map = new ModelTexture("textures/treemap.png");
    treemapTexture = map;

    treemapWidth = map->width;
    treemapHeight = map->height;

    treemapPixels.resize(map->width * map->height);
    glBindTexture(GL_TEXTURE_2D, map->textureID);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, treemapPixels.data());
    glBindTexture(GL_TEXTURE_2D, 0);

    std::vector<glm::mat4> emptyList;
    ModelTexture* texture = new ModelTexture("textures/texture_forest_solid.png");
    ShaderProgram* shader = new ShaderProgram("trees.vert", "trees.frag");

    if (i == 1)
        gameObjects.details = InstanceGroup(emptyList, shader, texture, i);
    else if (i == 0)
        gameObjects.trees = InstanceGroup(emptyList, shader, texture, i);
    else {
        ModelTexture* torchTex = new ModelTexture("textures/torch.png");
        gameObjects.torches = InstanceGroup(emptyList, shader, torchTex, i);
    }
}

void initMoonSun() {
    MeshGeometry* mesh = new MeshGeometry();
    mesh->generateSimpleSphereMesh();
    CHECK_GL_ERROR();

    ShaderProgram* shader = new ShaderProgram("skyobject.vert", "skyobject.frag");
    ModelTexture* sun = new ModelTexture("textures/sun.png");
    ModelTexture* moon = new ModelTexture("textures/moon.png");

    CHECK_GL_ERROR();

    gameObjects.sunSphere = Light(mesh, shader, sun, glm::vec3(243, 229, 188));
    gameObjects.moonSphere = Light(mesh, shader, moon, glm::vec3(216.0, 216.0, 216.0));
    gameObjects.moonSphere.isMoon = true;
}

void initCross() {
    MeshGeometry* mesh = new MeshGeometry();
    mesh->createCross();

    ShaderProgram* shader = new ShaderProgram("cross.vert", "cross.frag");
    ModelTexture* texture = new ModelTexture();
    CHECK_GL_ERROR();

    gameObjects.cross = Object(mesh, shader);
}

void initTerrain() {
    ModelTexture grass = ModelTexture(
        "textures/grassB.png",
        "textures/grassN.png",
        "textures/grassR.png",
        "textures/grassH.png",
        "textures/grassO.png");

    ModelTexture sand = ModelTexture(
        "textures/sandB.png",
        "textures/sandN.png",
        "textures/sandR.png",
        "textures/sandH.png",
        "textures/sandO.png");

    ModelTexture tiles = ModelTexture(
        "textures/tilesB.png",
        "textures/tilesN.png",
        "textures/tilesR.png",
        "textures/tilesH.png",
        "textures/tilesO.png");

    ModelTexture water = ModelTexture(
        "textures/waterB.png",
        "textures/waterN.png",
        "textures/waterR.png",
        "textures/waterH.png",
        "textures/waterO.png");


    TexturePack pack = TexturePack(grass, sand, water, tiles);
    gameObjects.terrain = new Terrain(0, 0, pack, new ModelTexture("textures/blendmapD.png"));
}

void restartGame() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    cleanUpObjects();
    gameState.elapsedTime = sylva::elapsedSeconds();

    gameObjects.skybox = new Skybox("skybox.vert", "skybox.frag");

    initTerrain();
    initPlayer();

    initInstances(0);
    initInstances(1);
    initInstances(2);

    initMoonSun();
    initCross();


    glm::vec3 playerPos = gameObjects.player->position;
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dz = -1; dz <= 1; ++dz) {
            generateInstancesNear(playerPos.x + dx * CHUNK_SIZE, playerPos.z + dz * CHUNK_SIZE, 0);
            generateInstancesNear(gameObjects.player->position.x, gameObjects.player->position.z, 1);
        }
    }

    int w = gameState.windowWidth;
    int h = gameState.windowHeight;

    if (h == 0)
        h = 1;

    glm::vec3 camPosition = gameObjects.player->position;
    glm::vec3 direction = gameObjects.player->direction;
    float angle = gameObjects.player->viewAngle;
    mainCamera = Camera(w, h, camPosition, direction, angle);
}

void initApp() {
    srand((unsigned int)time(NULL));

    glClearColor(0.5f, 0.4f, 0.8f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    restartGame();
}

void finalizeApp() {
    if (gameObjects.skybox) {
        delete gameObjects.skybox;
        gameObjects.skybox = nullptr;
    }

    cleanUpObjects();
}

void addTorch() {
    int centerX = gameState.windowWidth / 2;
    int centerY = gameState.windowHeight / 2;

    glm::vec3 rayOrigin = mainCamera.position;
    glm::vec3 rayDirection = mainCamera.getMouseRay(centerX, centerY, gameState.windowWidth, gameState.windowHeight);

    glm::vec3 current = rayOrigin;
    glm::vec3 position;

    int i = 0;
    while (i <= 1000) {
        current += rayDirection * 0.1f;
        float terrainHeight = gameObjects.terrain->getHeight(current.x, current.z);

        if (current.y <= terrainHeight) {
            std::cout << "Hit terrain at: " << current.x << ", " << terrainHeight << ", " << current.z << std::endl;
            position = glm::vec3(current.x, terrainHeight, current.z);
            break;
        }
        ++i;
    }

    if (i >= 1000)
        return;

    std::map<int, std::vector<glm::mat4>> newMatrices;

    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), position) *
        glm::rotate(glm::mat4(1.0f), glm::radians(270.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
        glm::scale(glm::mat4(1.0f), glm::vec3(0.40f));

    newMatrices[0].push_back(modelMatrix);
    placedPositions.push_back(position);
    collisionSpheres.emplace_back(position, 1.5);

    gameObjects.torches.addInstances(newMatrices);
}

void changeTree(glm::vec3 treePos, glm::mat4& mat) {
    std::map<int, std::vector<glm::mat4>> newMatrices;

    int size = gameObjects.trees.meshes.size();
    int index = rand() % size;

    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), treePos) *
        glm::rotate(glm::mat4(1.0f), glm::radians(static_cast<float>(rand() % 360)), glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::scale(glm::mat4(1.0f), glm::vec3(2.0f));

    newMatrices[index].push_back(modelMatrix);
    placedPositions.push_back(treePos);
    collisionSpheres.emplace_back(treePos, 1.5f);

    gameObjects.trees.addInstances(newMatrices);
}

static void cycleCameraView() {
    int current = gameState.cameraIndex;
    int nextIndex = (current + 1) % 4;

    if ((current == 2 && nextIndex == 3) || (current == 3 && nextIndex == 2)) {
        gameState.cameraIndex = nextIndex;
        return;
    }

    gameState.cameraStartPos = mainCamera.position;
    gameState.cameraStartLookAt = mainCamera.position + mainCamera.direction;

    glm::vec3 center = gameObjects.player->position;
    glm::vec3 targetPos, targetDir, targetLookAt;

    if (nextIndex == 0) {
        targetPos = center + glm::vec3(0.0f, 4.5f, 0.0f) + 1.75f * gameObjects.player->direction;

        targetDir = glm::normalize(gameObjects.player->direction);
        targetLookAt = targetPos + targetDir;

        if (glm::length(targetDir) < 0.001f)
            targetDir = glm::vec3(0.0f, 0.0f, -1.0f);
    }
    else if (nextIndex == 1) {
        float pitch = mainCamera.pitch;
        float distance = mainCamera.distanceFromPlayer;
        float horizontalDist = distance * cos(glm::radians(pitch));
        float verticalDist = distance * sin(glm::radians(pitch));
        float theta = glm::radians(mainCamera.angleAroundPlayer);

        float offsetX = horizontalDist * sin(theta);
        float offsetZ = horizontalDist * cos(theta);

        targetPos = center - glm::vec3(offsetX, -verticalDist, offsetZ);
        targetDir = glm::normalize(center - targetPos);
        targetLookAt = center;
    }
    else if (nextIndex == 2 || nextIndex == 3) {
        float t = gameState.currentSplineT;

        std::vector<glm::vec3> points = {
            center + glm::vec3(-20, 80, 0),
            center + glm::vec3(0, 90, 20),
            center + glm::vec3(20, 80, 0),
            center + glm::vec3(0, 90, -20),
            center + glm::vec3(-20, 80, 0),
            center + glm::vec3(0, 90, 20),
            center + glm::vec3(20, 80, 0),
            center + glm::vec3(0, 90, -20)
        };

        int segmentCount = points.size() - 3;
        float splineT = t * segmentCount;

        int i = glm::clamp((int)splineT, 0, segmentCount - 1);
        float localT = splineT - i;

        glm::vec3 p0 = points[i];
        glm::vec3 p1 = points[i + 1];
        glm::vec3 p2 = points[i + 2];
        glm::vec3 p3 = points[i + 3];

        targetPos = catmullRomSpline(p0, p1, p2, p3, localT);
        targetDir = glm::normalize(center - targetPos);
        targetLookAt = center;
    }

    gameState.cameraTargetIndex = nextIndex;
    gameState.cameraTargetPos = targetPos;
    gameState.cameraTargetDir = targetDir;
    gameState.cameraTargetLookAt = targetLookAt;

    gameState.cameraTransitionTime = 0.0f;
    gameState.isCameraTransitioning = true;
}

static void handleCharacterKey(char keyPressed) {
    switch (keyPressed) {
    case 'r':
        restartGame();
        break;
    case 'v':
        cycleCameraView();
        break;
    case 'c':
        gameObjects.player->state = (gameObjects.player->state == WAVING) ? IDLE : WAVING;
        gameObjects.player->animationTime = 0.0f;
        break;
    case 'm':
        gameState.cursor = !gameState.cursor;
        glfwSetInputMode(gWindow, GLFW_CURSOR,
            gameState.cursor ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
        break;
    case 'o':
        mainCamera.pitch = 30.0f;
        break;
    case 'p':
        mainCamera.distanceFromPlayer = 20.0f;
        break;
    case 'l':
        gameState.flashlightOn = !gameState.flashlightOn;
        std::cout << "flashlight: " << gameState.flashlightOn << std::endl;
        break;
    case 't':
        addTorch();
        break;
    case 'b':
        gameState.isBoy = !gameState.isBoy;
        if (gameState.isBoy) {
            std::string name = "models/boy/runA.fbx";
            gameObjects.player->geometry = new MeshGeometry(name);
        }
        else {
            std::string name = "models/girl/runA.fbx";
            gameObjects.player->geometry = new MeshGeometry(name);
        }
        break;
    }

    updatePlayerAndCamera();
}

static void keyCallback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
    const bool pressed = (action == GLFW_PRESS || action == GLFW_REPEAT);
    const bool released = (action == GLFW_RELEASE);

    switch (key) {
    // Arrow keys
    case GLFW_KEY_UP: gameState.keyMap[KEY_UP_ARROW] = pressed;
        return;
    case GLFW_KEY_DOWN: gameState.keyMap[KEY_DOWN_ARROW] = pressed;
        return;
    case GLFW_KEY_LEFT: gameState.keyMap[KEY_LEFT_ARROW] = pressed;
        return;
    case GLFW_KEY_RIGHT: gameState.keyMap[KEY_RIGHT_ARROW] = pressed;
        return;
    // WASD
    case GLFW_KEY_W: gameState.keyMap[KEY_UP_ARROW] = pressed;
        return;
    case GLFW_KEY_S: gameState.keyMap[KEY_DOWN_ARROW] = pressed;
        return;
    case GLFW_KEY_A: gameState.keyMap[KEY_LEFT_ARROW] = pressed;
        return;
    case GLFW_KEY_D: gameState.keyMap[KEY_RIGHT_ARROW] = pressed;
        return;
    default: break;
    }

    if (released) return;
    if (action != GLFW_PRESS) return;

    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        return;
    }

    if (key == GLFW_KEY_TAB) {
        gameState.advanceTime = !gameState.advanceTime;
        return;
    }

    // Map printable ASCII to the original keyboard handler.
    if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z) {
        char lower = static_cast<char>('a' + (key - GLFW_KEY_A));
        handleCharacterKey(lower);
    }
}

static void cursorPosCallback(GLFWwindow* /*window*/, double rawX, double rawY) {
    if (!gHaveLastRawMouse) {
        gLastRawMouseX = rawX;
        gLastRawMouseY = rawY;
        gHaveLastRawMouse = true;
        return;
    }

    double deltaX = rawX - gLastRawMouseX;
    double deltaY = rawY - gLastRawMouseY;
    gLastRawMouseX = rawX;
    gLastRawMouseY = rawY;

    if (gameState.cursor) return;

    int centerX = gameState.windowWidth / 2;
    int centerY = gameState.windowHeight / 2;

    // Feed the existing pitch/yaw update with the synthesised "absolute" pos.
    int x = centerX + static_cast<int>(deltaX);
    int y = centerY + static_cast<int>(deltaY);

    lastMouseX = xOffset = x - gameState.windowWidth / 2;
    lastMouseY = yOffset = y - gameState.windowHeight / 2;

    float dx = (float)(x - centerX);
    float dy = (float)(centerY - y);

    dx *= PLAYER_MOUSE_SENSITIVITY;
    dy *= PLAYER_MOUSE_SENSITIVITY;

    glm::vec3 lookDir(0.0f);
    if (mainCamera.firstPerson && gameObjects.player) {
        gameObjects.player->viewAngle -= dx;
        gameObjects.player->pitch += dy;

        gameObjects.player->pitch = glm::clamp(gameObjects.player->pitch, -89.0f, 89.0f);

        float yaw = glm::radians(gameObjects.player->viewAngle);
        float pitch = glm::radians(gameObjects.player->pitch);

        lookDir.x = cos(pitch) * -sin(yaw);
        lookDir.y = sin(pitch);
        lookDir.z = -cos(pitch) * cos(yaw);

        gameObjects.player->direction = glm::normalize(lookDir);
    }
    else {
        if (rightMouseDown) {
            mainCamera.rotateAngleAround(-dx);
            mainCamera.rotatePitch(-dy);
        }

        if (leftMouseDown) {
            mainCamera.zoom(dy * 0.5f);
        }
    }

    if (gameObjects.player)
        mainCamera.update(gameObjects.player, -1, -1);
    if (glm::length(lookDir) > 0.0001f)
        mainCamera.direction = glm::normalize(lookDir);
}

static void mouseButtonCallback(GLFWwindow* window, int button, int action, int /*mods*/) {
    if (button == GLFW_MOUSE_BUTTON_LEFT)
        leftMouseDown = (action == GLFW_PRESS);
    else if (button == GLFW_MOUSE_BUTTON_RIGHT)
        rightMouseDown = (action == GLFW_PRESS);

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double mx, my;
        glfwGetCursorPos(window, &mx, &my);
        int x = static_cast<int>(mx);
        int y = static_cast<int>(my);

        glm::vec3 rayOrigin = mainCamera.position;
        glm::vec3 rayDirection = mainCamera.getMouseRay(x, y, gameState.windowWidth, gameState.windowHeight);

        for (int meshIndex = 0; meshIndex < (int)gameObjects.trees.instanceMatrices.size(); ++meshIndex) {
            auto size = gameObjects.trees.instanceMatrices[meshIndex].size();
            for (int k = 0; k < (int)size; ++k) {
                glm::mat4 mat = gameObjects.trees.instanceMatrices[meshIndex][k];
                glm::vec3 treePos = glm::vec3(mat[3]);

                glm::vec3 halfExtents(1.0f, 10.0f, 1.0f);
                glm::vec3 boxMin = treePos - halfExtents;
                glm::vec3 boxMax = treePos + halfExtents;

                if (mainCamera.rayIntersectsAABB(rayOrigin, rayDirection, boxMin, boxMax)) {
                    gameObjects.trees.instanceMatrices[meshIndex].erase(
                        gameObjects.trees.instanceMatrices[meshIndex].begin() + k);
                    gameObjects.trees.updateInstanceBuffer(meshIndex);

                    changeTree(treePos, mat);
                    return;
                }
            }
        }
    }
}

static void scrollCallback(GLFWwindow* /*window*/, double /*xoffset*/, double yoffset) {
    if (!mainCamera.firstPerson) {
        mainCamera.zoom(static_cast<float>(yoffset) * 2.0f);
    }
    updatePlayerAndCamera();
}

static void glfwErrorCallback(int code, const char* description) {
    std::fprintf(stderr, "[GLFW] error %d: %s\n", code, description);
}

int main(int argc, char** argv) {
    glfwSetErrorCallback(glfwErrorCallback);

    if (!glfwInit())
        sylva::dieWithError("glfwInit failed");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, sylva::OGL_VER_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, sylva::OGL_VER_MINOR);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);

    gWindow = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Sylva", nullptr, nullptr);
    if (!gWindow) {
        glfwTerminate();
        sylva::dieWithError("glfwCreateWindow failed");
    }

    glfwMakeContextCurrent(gWindow);
    glfwSwapInterval(1);

    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress))
        sylva::dieWithError("Failed to load OpenGL (glad). OpenGL 3.3 core required.");

    int fbW = 0, fbH = 0;
    glfwGetFramebufferSize(gWindow, &fbW, &fbH);
    gameState.windowWidth = fbW;
    gameState.windowHeight = fbH;
    glViewport(0, 0, fbW, fbH);

    glfwSetFramebufferSizeCallback(gWindow, framebufferSizeCallback);
    glfwSetKeyCallback(gWindow, keyCallback);
    glfwSetMouseButtonCallback(gWindow, mouseButtonCallback);
    glfwSetCursorPosCallback(gWindow, cursorPosCallback);
    glfwSetScrollCallback(gWindow, scrollCallback);

    glfwSetCursorPos(gWindow, fbW / 2.0, fbH / 2.0);
    glfwSetInputMode(gWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(gWindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    glfwShowWindow(gWindow);
    glfwFocusWindow(gWindow);

    initApp();

    while (!glfwWindowShouldClose(gWindow)) {
        renderFrame();
        glfwPollEvents();
    }

    finalizeApp();

    glfwDestroyWindow(gWindow);
    glfwTerminate();
    return 0;
}
