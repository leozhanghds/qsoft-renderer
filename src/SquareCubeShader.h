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

    std::unordered_map<std::string, std::any> vertexShader(glm::vec4 &gl_Position) override;
    void fragmentShader() override;
};

inline std::unordered_map<std::string, std::any> SquareShader::vertexShader(glm::vec4 &gl_Position)
{
}

inline void SquareShader::fragmentShader()
{
}

class CubeShader : public Shader
{
public:
    CubeShader(int layoutCount) : Shader(layoutCount) {}
    ~CubeShader() = default;

    std::unordered_map<std::string, std::any> vertexShader(glm::vec4 &gl_Position) override;
    void fragmentShader() override;
};

inline std::unordered_map<std::string, std::any> CubeShader::vertexShader(glm::vec4 &gl_Position)
{
    // 从布局中获取顶点属性
    glm::vec3 pos = layout<glm::vec3>(0);
    glm::vec4 color = layout<glm::vec4>(1);

    //std::cout << "pos: " << pos.x << " " << pos.y << " " << pos.z << std::endl;

    // 处理顶点
    glm::mat4 mvp = getUniform<glm::mat4>("viewProjectionModelMatrix");
    gl_Position = glm::vec4(mvp * glm::vec4(pos, 1.0f));

    // 处理其他属性，写入缓存

    // 返回
    std::unordered_map<std::string, std::any> out;
    out["color"] = color;
    return out;
}

inline void CubeShader::fragmentShader()
{
}

#endif // SQUARE_SHADER_H
