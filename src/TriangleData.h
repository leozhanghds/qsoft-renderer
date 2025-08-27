#ifndef TRIANGLE_DATA_H
#define TRIANGLE_DATA_H

#pragma once

#include <array>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

class Triangle
{
public:
    Triangle(glm::vec4 v1, glm::vec4 v2, glm::vec4 v3)
    {
        _position[0] = v1;
        _position[1] = v2;
        _position[2] = v3;
    }
    // vertex
    std::array<glm::vec4, 3> _position{};

    // ndc_position
    std::array<glm::vec3, 3> _ndc_position{};

    // screen_position
    std::array<glm::vec2, 3> _screen_position{};

    // // color
    // bool _hasColor = false;
    // std::array<glm::vec4, 3> _color{};

    // // texture
    // bool _hasUV = false;
    // std::array<glm::vec2, 3> _uv{};

    // // normal
    // bool _hasNormal = false;
    // std::array<glm::vec3, 3> _normal{};
};

#endif // TRIANGLE_DATA_H