/**
 * @file noise.h
 * @brief Contains declarations for Perlin noise generation functions.
 */

#ifndef NOISE_H
#define NOISE_H

#include <cmath>

 /**
  * @brief Fade function used for smoothing in Perlin noise.
  * @param t Input value.
  * @return Smoothed output.
  */
float fade(float t);

/**
 * @brief Performs linear interpolation between two values.
 * @param a First value.
 * @param b Second value.
 * @param t Interpolation factor.
 * @return Interpolated result.
 */
float lerp(float a, float b, float t);

/**
 * @brief Gradient function for Perlin noise based on hashed value.
 * @param hash Hash value.
 * @param x X-coordinate.
 * @param y Y-coordinate.
 * @return Gradient result.
 */
float grad(int hash, float x, float y);

/**
 * @brief Computes 2D Perlin noise at specified coordinates.
 * @param x X-coordinate.
 * @param y Y-coordinate.
 * @return Noise value in range [-1, 1].
 */
float perlin(float x, float y);

#endif // NOISE_H
