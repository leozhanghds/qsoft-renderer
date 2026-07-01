#ifndef BUNNY_SHADER_H
#define BUNNY_SHADER_H

#include "Shader.h"

class BunnyShader : public Shader
{
public:
    BunnyShader(int layoutCount) : Shader(layoutCount) {}
    ~BunnyShader() = default;

    void vertexShader(glm::vec4 &gl_Position) override;
    void fragmentShader(glm::vec4 &gl_FragColor) override;
};

inline void BunnyShader::vertexShader(glm::vec4 &gl_Position)
{
    auto pos = vlayoutIn<glm::vec3>(0);
    auto normal = vlayoutIn<glm::vec3>(1);

    glm::mat4 model = getUniform<glm::mat4>("modelMatrix");
    glm::mat4 vp = getUniform<glm::mat4>("viewProjectionMatrix");
    gl_Position = vp * model * glm::vec4(pos, 1.0f);

    // 法线矩阵 = model 左上 3×3 的逆转置，将法线从模型空间变换到世界空间
    glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
    glm::vec3 worldNormal = glm::normalize(normalMatrix * normal);

    //vlayoutOut(0, Interpolation::Flat, normal);
    vlayoutOut(0, Interpolation::Flat, worldNormal);
}

inline void BunnyShader::fragmentShader(glm::vec4 &gl_FragColor)
{
    auto normal = flayoutIn<glm::vec3>(0);
    glm::vec3 n = glm::normalize(normal);
    gl_FragColor = glm::vec4(n * 0.5f + 0.5f, 1.0f);
}

#endif
