/**
 * @file noise.cpp
 * @brief Implementation of Perlin noise generation functions.
 */

#include "noise.h"

// Permutation table for Perlin noise
static const int p[512] = {
    151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,
    8,99,37,240,21,10,23,190,6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,
    35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,74,165,71,
    134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,
    55,46,245,40,244,102,143,54,65,25,63,161,1,216,80,73,209,76,132,187,208,89,
    18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,64,52,217,226,
    250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,
    189,28,42,223,183,170,213,119,248,152,2,44,154,163,70,221,153,101,155,167,43,
    172,9,129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,97,
    228,251,34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,
    107,49,192,214,31,181,199,106,157,184,84,204,176,115,121,50,45,127,4,150,254,
    138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
    // Repeat
    151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,
    8,99,37,240,21,10,23,190,6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,
    35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,74,165,71,
    134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,
    55,46,245,40,244,102,143,54,65,25,63,161,1,216,80,73,209,76,132,187,208,89,
    18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,64,52,217,226,
    250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,
    189,28,42,223,183,170,213,119,248,152,2,44,154,163,70,221,153,101,155,167,43,
    172,9,129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,97,
    228,251,34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,
    107,49,192,214,31,181,199,106,157,184,84,204,176,115,121,50,45,127,4,150,254,
    138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
};

/**
 * @brief Fade function used for smoothing in Perlin noise.
 * @param t Input value.
 * @return Smoothed output using 6t^5 - 15t^4 + 10t^3.
 */
float fade(float t) {
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

/**
 * @brief Performs linear interpolation between two values.
 * @param a First value.
 * @param b Second value.
 * @param t Interpolation factor [0, 1].
 * @return Interpolated result.
 */
float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

/**
 * @brief Gradient function for Perlin noise based on hashed value.
 * @param hash Hash value determining gradient direction.
 * @param x X-coordinate.
 * @param y Y-coordinate.
 * @return Dot product of gradient and distance vector.
 */
float grad(int hash, float x, float y) {
    int h = hash & 7;
    float u = h < 4 ? x : y;
    float v = h < 4 ? y : x;
    return ((h & 1) ? -u : u) + ((h & 2) ? -2.0f * v : 2.0f * v);
}

/**
 * @brief Computes 2D Perlin noise at specified coordinates.
 * @param x X-coordinate.
 * @param y Y-coordinate.
 * @return Noise value in range [-1, 1].
 */
float perlin(float x, float y) {
    // Find unit grid cell containing point
    int X = (int)floor(x) & 255;
    int Y = (int)floor(y) & 255;

    // Get relative xy coordinates of point within cell
    x -= floor(x);
    y -= floor(y);

    // Compute fade curves for x and y
    float u = fade(x);
    float v = fade(y);

    // Hash coordinates of the 4 square corners
    int A = p[X] + Y;
    int AA = p[A];
    int AB = p[A + 1];
    int B = p[X + 1] + Y;
    int BA = p[B];
    int BB = p[B + 1];

    // Blend results from 4 corners
    float res = lerp(
        lerp(grad(p[AA], x, y), grad(p[BA], x - 1, y), u),
        lerp(grad(p[AB], x, y - 1), grad(p[BB], x - 1, y - 1), u),
        v
    );

    return res;
}