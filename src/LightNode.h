#ifndef LIGHT_NODE_H
#define LIGHT_NODE_H

#pragma once

#include "Node.h"
#include "Shader.h"

#include <glm/glm.hpp>

#include "render_export.h"

// 光源立方体着色器：自发光恒定色，不受场景光照影响
class RENDER_EXPORT LightCubeShader : public Shader
{
public:
    LightCubeShader(int layoutCount) : Shader(layoutCount) {}
    ~LightCubeShader() = default;

    void vertexShader(glm::vec4 &gl_Position) override;
    void fragmentShader(glm::vec4 &gl_FragColor) override;
};

inline void LightCubeShader::vertexShader(glm::vec4 &gl_Position)
{
    auto pos = vlayoutIn<glm::vec3>(0);

    glm::mat4 mvp = getUniform<glm::mat4>("viewProjectionModelMatrix");
    gl_Position = mvp * glm::vec4(pos, 1.0f);

    vlayoutOut(0, Interpolation::Smooth, getUniform<glm::vec3>("lightColor"));
}

inline void LightCubeShader::fragmentShader(glm::vec4 &gl_FragColor)
{
    gl_FragColor = glm::vec4(getUniform<glm::vec3>("lightColor"), 1.0f);
}

// 光源节点：独立的场景节点，位置/颜色由渲染器同步到各着色器（lightPos / lightColor）
class RENDER_EXPORT LightNode : public Node
{
public:
    LightNode();
    virtual ~LightNode() = default;

    // 光源世界位置（model matrix 平移列）
    glm::vec3 getPosition() const;
    void setPosition(const glm::vec3 &pos);

    // 光源颜色
    glm::vec3 getLightColor() const { return _lightColor; }
    void setLightColor(const glm::vec3 &color) { _lightColor = color; }

private:
    void setupCube();

    glm::vec3 _lightColor{1.0f, 1.0f, 0.0f}; // 默认亮黄色
};

#endif // LIGHT_NODE_H
