#ifndef SQUARE_SHADER_H
#define SQUARE_SHADER_H

#include "Shader.h"
#include <unordered_map>

#include <iostream>

class SquareShader : public Shader
{
public:
    SquareShader(int layoutCount) : Shader(layoutCount) {}
    ~SquareShader() = default;

    size_t getOutLayoutCount() { return 4; }

    std::unordered_map<std::string, std::any> vertexShader(glm::vec4 &gl_Position) override;
    void fragmentShader(glm::vec4 &gl_FragColor) override;
};

inline std::unordered_map<std::string, std::any> SquareShader::vertexShader(glm::vec4 &gl_Position)
{
}

inline void SquareShader::fragmentShader(glm::vec4 &gl_FragColor)
{
}

class CubeShader : public Shader
{
public:
    CubeShader(int layoutCount) : Shader(layoutCount) {}
    ~CubeShader() = default;

    std::unordered_map<std::string, std::any> vertexShader(glm::vec4 &gl_Position) override;
    void fragmentShader(glm::vec4 &gl_FragColor) override;

    size_t getOutLayoutCount(){ return 1; }
};

inline std::unordered_map<std::string, std::any> CubeShader::vertexShader(glm::vec4 &gl_Position)
{
    // 从布局中获取顶点属性
    const glm::vec3& pos = vlayoutIn<glm::vec3>(0);
    const glm::vec4& color = vlayoutIn<glm::vec4>(1);

    // std::cout << "deal begin pos: " << pos.x << " " << pos.y << " " << pos.z << std::endl;

    // 处理顶点
    glm::mat4 mvp = getUniform<glm::mat4>("viewProjectionModelMatrix");
    gl_Position = glm::vec4(mvp * glm::vec4(pos, 1.0f));

    // std::cout << "deal end pos: " << gl_Position.x << " " << gl_Position.y << " " << gl_Position.z << std::endl;

    // 处理其他属性，写入缓存

    // 返回
    vlayoutOut(0, Interpolation::Smooth, color);

    std::unordered_map<std::string, std::any> out;
    out["color"] = color;
    return out;
}

inline void CubeShader::fragmentShader(glm::vec4 &gl_FragColor)
{
    const glm::vec4& color = flayoutIn<glm::vec4>(0);

    gl_FragColor = color;
}

#endif // SQUARE_SHADER_H
