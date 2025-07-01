#pragma once

#include <glm/glm.hpp>
#include <cmath>

class NoiseUtils {
public:
    static float Hash(float n) {
        return glm::fract(sin(n) * 43758.5453123f);
    }

    static float Noise(float x, float z) {
        int xi = static_cast<int>(floor(x));
        int zi = static_cast<int>(floor(z));
        float xf = x - xi;
        float zf = z - zi;

        float topLeft = Hash(xi + zi * 57);
        float topRight = Hash(xi + 1 + zi * 57);
        float bottomLeft = Hash(xi + (zi + 1) * 57);
        float bottomRight = Hash(xi + 1 + (zi + 1) * 57);

        float u = xf * xf * (3.0f - 2.0f * xf);
        float v = zf * zf * (3.0f - 2.0f * zf);

        float top = topLeft * (1 - u) + topRight * u;
        float bottom = bottomLeft * (1 - u) + bottomRight * u;

        return top * (1 - v) + bottom * v;
    }
};
