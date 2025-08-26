#ifndef TRIANGLE_DATA_H
#define TRIANGLE_DATA_H

#pragma once

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
    glm::vec4 _position[3];

    // color
    glm::vec4 _color[3]{};

    // texture
    glm::vec2 _uv[3]{};

    // ndc_position
    glm::vec3 _ndc_position[3]{};

    // screen_position
    glm::vec2 _screen_position[3]{};
};

#endif // TRIANGLE_DATA_H