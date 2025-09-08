#ifndef SQUARE_SHADER_H
#define SQUARE_SHADER_H

#include "Shader.h"

#include <iostream>

class SquareShader : public Shader
{
public:
    SquareShader(int layoutCount) : Shader(layoutCount) {}
    ~SquareShader() = default;

    void vertexShader(glm::vec4 &gl_Position) override;
    void fragmentShader(glm::vec4 &gl_FragColor) override;
};

inline void SquareShader::vertexShader(glm::vec4 &gl_Position)
{
    // 从布局中获取顶点属性
    auto pos = vlayoutIn<glm::vec3>(0);
    auto color = vlayoutIn<glm::vec4>(1);
    auto uv = vlayoutIn<glm::vec2>(2);

    // 处理顶点
    glm::mat4 mvp = getUniform<glm::mat4>("viewProjectionModelMatrix");
    gl_Position = glm::vec4(mvp * glm::vec4(pos, 1.0f));

    vlayoutOut(0, Interpolation::Smooth, color);
    vlayoutOut(1, Interpolation::Smooth, uv);
}

inline void SquareShader::fragmentShader(glm::vec4 &gl_FragColor)
{
    auto color = flayoutIn<glm::vec4>(0);
    auto uv = flayoutIn<glm::vec2>(1);

    auto texture = getTexture(0);

    auto uvcolor = texture->sampleBilinear(uv);

    gl_FragColor = customBlend(uvcolor, color, 0.4, 0.6);
    // gl_FragColor = uvcolor;
}

class CubeShader : public Shader
{
public:
    CubeShader(int layoutCount) : Shader(layoutCount) {}
    ~CubeShader() = default;

    void vertexShader(glm::vec4 &gl_Position) override;
    void fragmentShader(glm::vec4 &gl_FragColor) override;
};

inline void CubeShader::vertexShader(glm::vec4 &gl_Position)
{
    // 从布局中获取顶点属性
    auto pos = vlayoutIn<glm::vec3>(0);
    auto color = vlayoutIn<glm::vec4>(1);

    // std::cout << "deal begin pos: " << pos.x << " " << pos.y << " " << pos.z << std::endl;

    // 处理顶点
    glm::mat4 mvp = getUniform<glm::mat4>("viewProjectionModelMatrix");
    gl_Position = glm::vec4(mvp * glm::vec4(pos, 1.0f));

    // std::cout << "deal end pos: " << gl_Position.x << " " << gl_Position.y << " " << gl_Position.z << std::endl;

    // 处理其他属性，写入缓存

    // 返回
    vlayoutOut(0, Interpolation::Smooth, color);
}

inline void CubeShader::fragmentShader(glm::vec4 &gl_FragColor)
{
    auto color = flayoutIn<glm::vec4>(0);

    gl_FragColor = color;
}

#endif // SQUARE_SHADER_H
