#ifndef GROUND_SHADER_H
#define GROUND_SHADER_H

#include "Shader.h"
#include "ShadowHelper.h"

// 地板着色器：纯色基色 + Blinn-Phong 光照 + 阴影采样（用于接收场景阴影）
class GroundShader : public Shader
{
public:
    GroundShader(int layoutCount) : Shader(layoutCount) {}
    ~GroundShader() = default;

    void vertexShader(glm::vec4 &gl_Position) override;
    void fragmentShader(glm::vec4 &gl_FragColor) override;
};

inline void GroundShader::vertexShader(glm::vec4 &gl_Position)
{
    auto pos = vlayoutIn<glm::vec3>(0);
    auto normal = vlayoutIn<glm::vec3>(1);

    glm::mat4 model = getUniform<glm::mat4>("modelMatrix");
    glm::mat4 vp = getUniform<glm::mat4>("viewProjectionMatrix");
    gl_Position = vp * model * glm::vec4(pos, 1.0f);

    // 法线矩阵：model 左上 3×3 的逆转置，法线从模型空间变换到世界空间
    glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
    glm::vec3 worldNormal = glm::normalize(normalMatrix * normal);

    glm::vec4 worldPos = model * glm::vec4(pos, 1.0f);

    vlayoutOut(0, Interpolation::Smooth, worldNormal);
    vlayoutOut(1, Interpolation::Smooth, worldPos);
}

inline void GroundShader::fragmentShader(glm::vec4 &gl_FragColor)
{
    auto normal = flayoutIn<glm::vec3>(0);
    normal = glm::normalize(normal);

    auto worldPos = flayoutIn<glm::vec4>(1);
    glm::vec3 fragPos(worldPos);

    // 地板基色（uniform 可调）
    glm::vec3 objectColor = getUniform<glm::vec3>("groundColor");

    // 光照参数
    glm::vec3 lightPos = getUniform<glm::vec3>("lightPos");
    glm::vec3 lightColor = getUniform<glm::vec3>("lightColor");
    glm::vec3 viewPos = getUniform<glm::vec3>("viewPos");

    glm::vec3 lightDir = glm::normalize(lightPos - fragPos);
    glm::vec3 viewDir = glm::normalize(viewPos - fragPos);
    glm::vec3 halfwayDir = glm::normalize(lightDir + viewDir);

    // 环境光
    glm::vec3 ambient = 0.4f * 0.6f * lightColor * objectColor;

    // 漫反射 (Lambert)
    float diff = glm::max(glm::dot(normal, lightDir), 0.0f);
    glm::vec3 diffuse = diff * lightColor * objectColor;

    // 镜面反射 (Blinn-Phong)
    float spec = glm::pow(glm::clamp(glm::dot(normal, halfwayDir), 0.0f, 1.0f), 64.0f);
    glm::vec3 specular = spec * lightColor;

    // 阴影
    float shadow = computeShadowFactor(*this, fragPos);

    gl_FragColor = glm::vec4(ambient + shadow * (diffuse + specular), 1.0f);
}

#endif // GROUND_SHADER_H
