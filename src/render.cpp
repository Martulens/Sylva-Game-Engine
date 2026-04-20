/**
 * @file render.cpp
 * @brief Contains rendering utilities and shader uniform setup for various scene objects.
 */

#include <iostream>
#include "render.h"
#include "sylva/sylva.h"
#include "data.h"
#include "camera.h"
#include "gamestate.h"
#include "terrain.h"
#include "shaders.h"

 /**
  * @brief Sets up shader uniforms for lighting, camera, and textures.
  *
  * @param shader Pointer to the shader program.
  * @param torches Reference to the instance group of torches.
  * @param light Reference to the main light source.
  * @param cam Reference to the active camera.
  * @param texture Pointer to the model texture.
  * @param gameState Reference to the global game state.
  * @param playerPos Position of the player.
  * @param playerDir Direction the player is facing.
  */
void setupUniforms(ShaderProgram* shader, InstanceGroup& torches, Light& light, Camera& cam, ModelTexture* texture, GameState& gameState, glm::vec3& playerPos, glm::vec3& playerDir) {
    glm::vec3 lightDirection = light.direction;
    glm::vec3 lightPosition = light.position;
    glm::vec3 lightColor = light.color;
    bool isNight = false;

    glUniformMatrix4fv(shader->projectionMatrix, 1, GL_FALSE, &cam.projectionMatrix[0][0]);
    glUniformMatrix4fv(shader->viewMatrix, 1, GL_FALSE, &cam.viewMatrix[0][0]);

    glUniform1f(shader->density, light.fogDensity);
    glUniform1f(shader->gradient, light.fogGradient);

    glUniform1f(shader->shineDamper, texture->shineDamper);
    glUniform1f(shader->reflectivity, texture->reflectivity);

    glUniform3f(shader->lightDirection, lightDirection.x, lightDirection.y, lightDirection.z);
    glUniform3f(shader->lightPosition, lightPosition.x, lightPosition.y, lightPosition.z);
    glUniform3f(shader->lightColour, light.color.r / 255.0f, light.color.g / 255.0f, light.color.b / 255.0f);
    glUniform1f(glGetUniformLocation(shader->program, "lightBrightness"), light.brightness);

    glUniform1i(glGetUniformLocation(shader->program, "isPointLight"), light.isMoon);

    glUniform3f(shader->skyColor, light.skyColor.x, light.skyColor.y, light.skyColor.z);

    int flashlight = gameState.flashlightOn;

    glUniform1i(shader->flashlightOn, flashlight);

    glUniform3fv(shader->flashlightPos, 1, glm::value_ptr(playerPos));
    glUniform3fv(shader->flashlightDir, 1, glm::value_ptr(playerDir));
    glUniform1f(shader->cutoffAngle, glm::radians(CUTOFF_ANGLE));

    int count = 0;
    std::vector<glm::vec3> lightPositions;
    for (int i = 0; i < torches.instanceMatrices.size(); ++i) {
        for (int j = 0; j < torches.instanceMatrices[i].size(); ++j) {
            if (count >= MAX_POINT_LIGHTS) break;
            glm::vec3 pos = glm::vec3(torches.instanceMatrices[i][j][3]);
            lightPositions.push_back(pos);
            count++;
        }
    }

    glUniform1i(glGetUniformLocation(shader->program, "numPointLights"), lightPositions.size());

    for (int i = 0; i < lightPositions.size(); ++i) {
        std::string uniformName = "pointLightPositions[" + std::to_string(i) + "]";
        glUniform3f(glGetUniformLocation(shader->program, uniformName.c_str()), lightPositions[i].x, lightPositions[i].y + 4.0f, lightPositions[i].z);

        uniformName = "pointLightColors[" + std::to_string(i) + "]";
        glUniform3f(glGetUniformLocation(shader->program, uniformName.c_str()), 1.0f, 0.5f, 0.2f);
    }

    float currentTime = sylva::elapsedSeconds();
    GLint timeLoc = glGetUniformLocation(shader->program, "time");
    glUniform1f(timeLoc, currentTime);
}

/**
 * @brief Binds terrain textures to their respective texture units and assigns them to shader uniforms.
 *
 * @param terrain Pointer to the terrain object.
 * @param shader Pointer to the shader program.
 * @param pack Reference to the terrain's texture pack.
 */
void setupTextures(Terrain* terrain, ShaderProgram* shader, TexturePack& pack) {
    // Back layer
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, terrain->texture.backTexture.baseColorID);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, terrain->texture.backTexture.normalID);
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, terrain->texture.backTexture.roughnessID);
    glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, terrain->texture.backTexture.heightID);
    glActiveTexture(GL_TEXTURE4); glBindTexture(GL_TEXTURE_2D, terrain->texture.backTexture.emissiveID);

    // Red layer
    glActiveTexture(GL_TEXTURE5); glBindTexture(GL_TEXTURE_2D, terrain->texture.rTexture.baseColorID);
    glActiveTexture(GL_TEXTURE6); glBindTexture(GL_TEXTURE_2D, terrain->texture.rTexture.normalID);
    glActiveTexture(GL_TEXTURE7); glBindTexture(GL_TEXTURE_2D, terrain->texture.rTexture.roughnessID);
    glActiveTexture(GL_TEXTURE8); glBindTexture(GL_TEXTURE_2D, terrain->texture.rTexture.heightID);
    glActiveTexture(GL_TEXTURE9); glBindTexture(GL_TEXTURE_2D, terrain->texture.rTexture.emissiveID);

    // Green layer
    glActiveTexture(GL_TEXTURE10); glBindTexture(GL_TEXTURE_2D, terrain->texture.gTexture.baseColorID);
    glActiveTexture(GL_TEXTURE11); glBindTexture(GL_TEXTURE_2D, terrain->texture.gTexture.normalID);
    glActiveTexture(GL_TEXTURE12); glBindTexture(GL_TEXTURE_2D, terrain->texture.gTexture.roughnessID);
    glActiveTexture(GL_TEXTURE13); glBindTexture(GL_TEXTURE_2D, terrain->texture.gTexture.heightID);
    glActiveTexture(GL_TEXTURE14); glBindTexture(GL_TEXTURE_2D, terrain->texture.gTexture.emissiveID);

    // Blue layer
    glActiveTexture(GL_TEXTURE15); glBindTexture(GL_TEXTURE_2D, terrain->texture.bTexture.baseColorID);
    glActiveTexture(GL_TEXTURE16); glBindTexture(GL_TEXTURE_2D, terrain->texture.bTexture.normalID);
    glActiveTexture(GL_TEXTURE17); glBindTexture(GL_TEXTURE_2D, terrain->texture.bTexture.roughnessID);
    glActiveTexture(GL_TEXTURE18); glBindTexture(GL_TEXTURE_2D, terrain->texture.bTexture.heightID);
    glActiveTexture(GL_TEXTURE19); glBindTexture(GL_TEXTURE_2D, terrain->texture.bTexture.emissiveID);

    // Blend map
    glActiveTexture(GL_TEXTURE20); glBindTexture(GL_TEXTURE_2D, terrain->blendMap->textureID);

    glUniform1i(glGetUniformLocation(terrain->shader->program, "backB"), 0);
    glUniform1i(glGetUniformLocation(terrain->shader->program, "backN"), 1);
    glUniform1i(glGetUniformLocation(terrain->shader->program, "backR"), 2);
    glUniform1i(glGetUniformLocation(terrain->shader->program, "backH"), 3);
    glUniform1i(glGetUniformLocation(terrain->shader->program, "backE"), 4);

    glUniform1i(glGetUniformLocation(terrain->shader->program, "redB"), 5);
    glUniform1i(glGetUniformLocation(terrain->shader->program, "redN"), 6);
    glUniform1i(glGetUniformLocation(terrain->shader->program, "redR"), 7);
    glUniform1i(glGetUniformLocation(terrain->shader->program, "redH"), 8);
    glUniform1i(glGetUniformLocation(terrain->shader->program, "redE"), 9);

    glUniform1i(glGetUniformLocation(terrain->shader->program, "greenB"), 10);
    glUniform1i(glGetUniformLocation(terrain->shader->program, "greenN"), 11);
    glUniform1i(glGetUniformLocation(terrain->shader->program, "greenR"), 12);
    glUniform1i(glGetUniformLocation(terrain->shader->program, "greenH"), 13);
    glUniform1i(glGetUniformLocation(terrain->shader->program, "greenE"), 14);

    glUniform1i(glGetUniformLocation(terrain->shader->program, "blueB"), 15);
    glUniform1i(glGetUniformLocation(terrain->shader->program, "blueN"), 16);
    glUniform1i(glGetUniformLocation(terrain->shader->program, "blueR"), 17);
    glUniform1i(glGetUniformLocation(terrain->shader->program, "blueH"), 18);
    glUniform1i(glGetUniformLocation(terrain->shader->program, "blueE"), 19);

    glUniform1i(glGetUniformLocation(terrain->shader->program, "blendMap"), 20);

}

/**
 * @brief Sets transformation matrix for the player character.
 *
 * @param p Pointer to the player object.
 */
void setupPlayerUniforms(Player* p) {
    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), p->position) *
        glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

    if (p->shader->transformationMatrix != -1) {
        glUniformMatrix4fv(p->shader->transformationMatrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));
    }

    glUniform1i(glGetUniformLocation(p->shader->program, "modelTexture"), 0);
}

/**
 * @brief Finds the appropriate keyframe index for animation interpolation.
 *
 * @param time Current animation time.
 * @param count Total number of keyframes.
 * @param keys Pointer to the array of animation keyframes.
 * @return Index of the keyframe.
 */
unsigned int findKeyframeIndex(double time, unsigned int count, const aiVectorKey* keys) {
    for (unsigned int i = 0; i < count - 1; ++i) {
        if (time < keys[i + 1].mTime)
            return i;
    }
    return count - 2;
}

/**
 * @brief Recursively reads and applies animation data to the node hierarchy.
 *
 * @param animationTime Current animation time.
 * @param node Pointer to the current animation node.
 * @param parentTransform Parent transformation matrix.
 * @param mesh Pointer to the mesh geometry.
 */
void readNodeHeirarchy(float animationTime, unsigned int animIndex, const aiNode* node, const glm::mat4& parentTransform, MeshGeometry* mesh) {
    std::string nodeName(node->mName.C_Str());

    glm::mat4 nodeTransform = glm::transpose(glm::make_mat4(&node->mTransformation.a1));

    if (mesh->scene->mNumAnimations == 0 || animIndex >= mesh->scene->mNumAnimations) {
        glm::mat4 globalTransform = parentTransform * nodeTransform;
        if (mesh->boneMapping.count(nodeName)) {
            int boneIndex = mesh->boneMapping[nodeName];
            mesh->boneFinalTransforms[boneIndex] = globalTransform * mesh->boneOffsets[boneIndex];
        }
        for (unsigned int i = 0; i < node->mNumChildren; ++i)
            readNodeHeirarchy(animationTime, animIndex, node->mChildren[i], globalTransform, mesh);
        return;
    }
    const aiAnimation* animation = mesh->scene->mAnimations[animIndex];
    const aiNodeAnim* nodeAnim = nullptr;

    for (unsigned int i = 0; i < animation->mNumChannels; ++i) {
        if (animation->mChannels[i]->mNodeName == node->mName) {
            nodeAnim = animation->mChannels[i];
            break;
        }
    }

    if (nodeAnim) {
        // Position
        glm::vec3 translation(0.0f);
        if (nodeAnim->mNumPositionKeys > 1) {
            for (unsigned int i = 0; i < nodeAnim->mNumPositionKeys - 1; ++i) {
                if (animationTime < nodeAnim->mPositionKeys[i + 1].mTime) {
                    float t1 = nodeAnim->mPositionKeys[i].mTime;
                    float t2 = nodeAnim->mPositionKeys[i + 1].mTime;
                    float factor = (animationTime - t1) / (t2 - t1);

                    aiVector3D start = nodeAnim->mPositionKeys[i].mValue;
                    aiVector3D end = nodeAnim->mPositionKeys[i + 1].mValue;
                    aiVector3D interpolated = start + factor * (end - start);

                    translation = glm::vec3(interpolated.x, interpolated.y, interpolated.z);
                    break;
                }
            }
        }

        // Rotation
        glm::quat rotation(1.0f, 0.0f, 0.0f, 0.0f);
        if (nodeAnim->mNumRotationKeys > 1) {
            for (unsigned int i = 0; i < nodeAnim->mNumRotationKeys - 1; ++i) {
                if (animationTime < nodeAnim->mRotationKeys[i + 1].mTime) {
                    float t1 = nodeAnim->mRotationKeys[i].mTime;
                    float t2 = nodeAnim->mRotationKeys[i + 1].mTime;
                    float factor = (animationTime - t1) / (t2 - t1);

                    aiQuaternion start = nodeAnim->mRotationKeys[i].mValue;
                    aiQuaternion end = nodeAnim->mRotationKeys[i + 1].mValue;
                    aiQuaternion interpolated;
                    aiQuaternion::Interpolate(interpolated, start, end, factor);
                    interpolated.Normalize();

                    rotation = glm::quat(interpolated.w, interpolated.x, interpolated.y, interpolated.z);
                    break;
                }
            }
        }

        // Scaling
        glm::vec3 scale(1.0f);
        if (nodeAnim->mNumScalingKeys > 1) {
            for (unsigned int i = 0; i < nodeAnim->mNumScalingKeys - 1; ++i) {
                if (animationTime < nodeAnim->mScalingKeys[i + 1].mTime) {
                    float t1 = nodeAnim->mScalingKeys[i].mTime;
                    float t2 = nodeAnim->mScalingKeys[i + 1].mTime;
                    float factor = (animationTime - t1) / (t2 - t1);

                    aiVector3D start = nodeAnim->mScalingKeys[i].mValue;
                    aiVector3D end = nodeAnim->mScalingKeys[i + 1].mValue;
                    aiVector3D interpolated = start + factor * (end - start);

                    scale = glm::vec3(interpolated.x, interpolated.y, interpolated.z);
                    break;
                }
            }
        }

        nodeTransform = glm::translate(glm::mat4(1.0f), translation) *
            glm::mat4_cast(rotation) *
            glm::scale(glm::mat4(1.0f), scale);
    }


    glm::mat4 globalTransform = parentTransform * nodeTransform;

    if (mesh->boneMapping.count(nodeName)) {
        int boneIndex = mesh->boneMapping[nodeName];
        mesh->boneFinalTransforms[boneIndex] = globalTransform * mesh->boneOffsets[boneIndex];
    }

    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        readNodeHeirarchy(animationTime, animIndex, node->mChildren[i], globalTransform, mesh);
    }
}

/**
 * @brief Draws a cross object in the center of the screen.
 *
 * @param cross Object representing the cross.
 */
void drawCross(Object cross) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(cross.shader->program);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(0.5f));

    GLint modelLoc = glGetUniformLocation(cross.shader->program, "transformationMatrix");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(cross.geometry->vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    glUseProgram(0);
    glDisable(GL_BLEND);  // Restore previous state
}

/**
 * @brief Renders the animated player character with appropriate uniforms.
 *
 * @param player Pointer to the player object.
 * @param torches Reference to the torch instance group.
 * @param cam Reference to the active camera.
 * @param light Reference to the active light.
 * @param gameState Reference to the global game state.
 */
void drawPlayer(Player* player, InstanceGroup& torches, Camera& cam, Light& light, GameState& gameState) {
    glUseProgram(player->shader->program);

    const unsigned int animIdx = (player->state == RUNNING)
        ? player->geometry->runAnimationIndex
        : player->geometry->animationIndex;
    readNodeHeirarchy(player->animationTime, animIdx, player->geometry->scene->mRootNode, glm::mat4(1.0f), player->geometry);
    setupUniforms(player->shader, torches, light, cam, player->texture, gameState, player->position, player->direction);

    float angle = atan2(player->renderFacingDirection.x, player->renderFacingDirection.z);
    glm::mat4 modelMatrix =
        glm::translate(glm::mat4(1.0f), player->position) *
        glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::scale(glm::mat4(1.0f), glm::vec3(0.05f));

    glUniformMatrix4fv(player->shader->transformationMatrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));
    GLint bonesUniform = glGetUniformLocation(player->shader->program, "bones");
    const int bonesToUpload = std::min(player->geometry->numBones, MAX_BONES);
    glUniformMatrix4fv(bonesUniform, bonesToUpload, GL_FALSE,
        glm::value_ptr(player->geometry->boneFinalTransforms[0]));


    glUniform1i(glGetUniformLocation(player->shader->program, "modelTexture"), 0); // Before draw call
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, player->texture->textureID);

    if (player->geometry->vao != 0) {
        player->draw();
    }
    else {
        std::cout << "[ERROR] Player geometry not initialized!" << std::endl;
    }

    glUseProgram(0);
}

/**
 * @brief Renders the terrain.
 *
 * @param terrain Pointer to the terrain.
 * @param torches Reference to torch instances.
 * @param cam Reference to camera.
 * @param light Reference to the sun/moon light.
 * @param gameState Reference to game state.
 * @param player Pointer to player for flashlight direction.
 */
void drawTerrain(Terrain* terrain, InstanceGroup& torches, Camera& cam, Light& light, GameState& gameState, Player* player) {
    glUseProgram(terrain->shader->program);

    setupUniforms(terrain->shader, torches, light, cam, terrain->blendMap, gameState, player->position, player->direction);
    setupTextures(terrain, terrain->shader, terrain->texture);

    if (terrain->geometry->vao != 0) {
        terrain->draw();
    }
    else {
        std::cout << "[ERROR] Terrain geometry not initialized!" << std::endl;
    }

    glUseProgram(0);
}

/**
 * @brief Renders the skybox with blending based on sun position.
 *
 * @param skybox Pointer to the skybox.
 * @param cam Reference to camera.
 * @param sunClamp Blend factor from sun.
 */
void drawSkybox(Skybox* skybox, Camera& cam, float sunClamp) {
    if (skybox) {
        glUseProgram(skybox->shader->program);
        GLint blendLoc = glGetUniformLocation(skybox->shader->program, "skyboxBlend");
        glUniform1f(blendLoc, sunClamp);
        glUseProgram(0);

        skybox->draw(cam.viewMatrix, cam.projectionMatrix);
    }
}

/**
 * @brief Renders all tree instances.
 *
 * @param tree Reference to instance group of trees.
 * @param torches Reference to torch instances.
 * @param cam Reference to camera.
 * @param light Reference to the main light.
 * @param gameState Reference to the game state.
 * @param player Pointer to player.
 */
void drawTrees(InstanceGroup& tree, InstanceGroup& torches, Camera& cam, Light& light, GameState& gameState, Player* player) {
    glUseProgram(tree.shader->program);
    setupUniforms(tree.shader, torches, light, cam, tree.texture, gameState, player->position, player->direction);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tree.texture->textureID);

    size_t size = tree.meshes.size();
    for (int i = 0; i < size; i++) {
        if (tree.countInstances[i] == 0)
            continue;

        MeshGeometry* mesh = tree.meshes[i];
        glBindVertexArray(mesh->vao);
        glDrawElementsInstanced(
            GL_TRIANGLES,
            mesh->numTriangles * 3,
            GL_UNSIGNED_INT,
            nullptr,
            tree.countInstances[i]
        );

        CHECK_GL_ERROR();

        glBindVertexArray(0);
    }
    glUseProgram(0);

}

/**
 * @brief Draws the sun or moon object.
 *
 * @param light Reference to the light (sun or moon).
 * @param torches Reference to torch instances.
 * @param cam Reference to the camera.
 * @param viewMatrix View matrix.
 * @param projectionMatrix Projection matrix.
 * @param isMoon True if rendering moon, false for sun.
 * @param gamestate Reference to the game state.
 * @param p Pointer to the player.
 */
void drawSun(Light& light, InstanceGroup& torches, Camera& cam, glm::mat4& viewMatrix, glm::mat4& projectionMatrix, bool isMoon, GameState& gamestate, Player* p) {
    glUseProgram(light.shader->program);

    setupUniforms(light.shader, torches, light, cam, light.texture, gamestate, p->position, p->direction);

    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), light.position)
        * glm::scale(glm::mat4(1.0f), glm::vec3(50.0f));

    glUniformMatrix4fv(light.shader->transformationMatrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));
    glUniformMatrix4fv(light.shader->viewMatrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(light.shader->projectionMatrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    // Light color
    glUniform3f(glGetUniformLocation(light.shader->program, "lightColour"),
        light.color.r / 255.0f, light.color.g / 255.0f, light.color.b / 255.0f);

    // Fake light dir — from camera toward object
    glUniform3f(glGetUniformLocation(light.shader->program, "fakeLightDir"), 0.0f, -1.0f, 0.0f);

    // Is it the moon?
    glUniform1i(glGetUniformLocation(light.shader->program, "isMoon"), isMoon);

    glUniform1i(glGetUniformLocation(light.shader->program, "modelTexture"), 0); // Before draw call
    glUniform1f(glGetUniformLocation(light.shader->program, "deltaTime"), gamestate.advanceTime);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, light.texture->textureID);

    glBindVertexArray(light.geometry->vao);
    glDrawElements(GL_TRIANGLES, light.geometry->numTriangles * 3, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glUseProgram(0);
}


