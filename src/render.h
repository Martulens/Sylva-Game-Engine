/**
 * @file render.h
 * @brief Declares rendering functions for terrain, trees, player, lights, skybox, and UI.
 */

#ifndef RENDER_H
#define RENDER_H

#include "sylva/sylva.h"
#include "objects.h"
#include "camera.h"
#include "meshgeometry.h"
#include "terrain.h"
#include "gamestate.h"

/**
 * @brief Draws the terrain with fog and lighting applied.
 * @param terrain Pointer to the terrain object.
 * @param torches Reference to torch instances.
 * @param cam Reference to the active camera.
 * @param light Light affecting the terrain.
 * @param gameState Reference to the current game state.
 * @param p Pointer to the player object for flashlight context.
 */
void drawTerrain(Terrain* terrain, InstanceGroup& torches, Camera& cam, Light& light, GameState& gameState, Player* p);

/**
 * @brief Draws instanced tree or detail groups.
 * @param trees Instance group to be rendered.
 * @param torches Torch instances for point lighting.
 * @param cam Reference to the active camera.
 * @param light Lighting affecting the trees.
 * @param gameState Reference to the current game state.
 * @param p Pointer to the player for lighting context.
 */
void drawTrees(InstanceGroup& trees, InstanceGroup& torches, Camera& cam, Light& light, GameState& gameState, Player* p);

/**
 * @brief Renders the player model with animation and lighting.
 * @param player Pointer to the player object.
 * @param torches Torch light instances.
 * @param cam Reference to the active camera.
 * @param light Reference to the light source.
 * @param gameState Reference to the game state.
 */
void drawPlayer(Player* player, InstanceGroup& torches, Camera& cam, Light& light, GameState& gameState);

/**
 * @brief Draws the sun or moon in the sky.
 * @param light Reference to the light object (sun or moon).
 * @param torches Torch instances for environmental light.
 * @param cam Reference to the active camera.
 * @param viewMatrix View matrix.
 * @param projectionMatrix Projection matrix.
 * @param isMoon True if rendering moon, false for sun.
 * @param gamestate Reference to the game state.
 * @param p Pointer to the player.
 */
void drawSun(Light& light, InstanceGroup& torches, Camera& cam, glm::mat4& viewMatrix, glm::mat4& projectionMatrix, bool isMoon, GameState& gamestate, Player* p);

/**
 * @brief Draws the cube-mapped skybox.
 * @param skybox Pointer to the skybox object.
 * @param cam Reference to the camera.
 * @param sunFactor Blend factor for sky brightness.
 */
void drawSkybox(Skybox* skybox, Camera& cam, float sunFactor);

/**
 * @brief Draws a simple UI crosshair or cursor in the center of the screen.
 * @param cross Object representing the cross.
 */
void drawCross(Object cross);

void drawText(const std::string& text, float x, float y, float scale, const glm::vec3& color);

#endif // RENDER_H
