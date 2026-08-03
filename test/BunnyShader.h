#ifndef BUNNY_SHADER_H
#define BUNNY_SHADER_H

#include "Shader.h"
#include "ShadowHelper.h"

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

    // 法线矩阵 = model 左上 3×3 的逆矩阵的转置矩阵，将法线从模型空间变换到世界空间
    glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
    glm::vec3 worldNormal = glm::normalize(normalMatrix * normal);

    glm::vec4 worldPos = model * glm::vec4(pos, 1.0f);

    vlayoutOut(0, Interpolation::Flat, normal);
    //vlayoutOut(0, Interpolation::Smooth, worldNormal);

    vlayoutOut(1, Interpolation::Smooth, worldPos);
}

inline void BunnyShader::fragmentShader(glm::vec4 &gl_FragColor)
{
    auto normal = flayoutIn<glm::vec3>(0);
    normal = glm::normalize(normal);

    auto normalColor = normal * 0.5f + 0.5f;

    if(0){
        gl_FragColor = glm::vec4(normalColor, 1.0f);
        return;
    }

    auto worldPos = flayoutIn<glm::vec4>(1);

    // 光源颜色 — 环境光、漫反射、镜面反射各自独立
    glm::vec3 lightAmbient  = glm::vec3(0.6f);  // 很暗的灰色
    glm::vec3 lightDiffuse  = glm::vec3(1.0f);  // 纯白（暖色调交给物体色）
    glm::vec3 lightSpecular = glm::vec3(1.0f);  // 纯白色高光

    // 兔子物体色
    //glm::vec3 objectColor = glm::vec3(0.82f, 0.78f, 0.73f);
    glm::vec3 objectColor = normalColor;

    if (0)
    {
        // 1. 原本颜色（无光照，纯物体色）
        gl_FragColor = glm::vec4(objectColor, 1.0f);
        return;
    }

    // ---- 以下分支需要光照向量 ----
    glm::vec3 lightPos = getUniform<glm::vec3>("lightPos");   // light
    glm::vec3 viewPos = getUniform<glm::vec3>("viewPos");     // eye
    glm::vec3 lightColor = getUniform<glm::vec3>("lightColor"); // 光源颜色（默认白）
    glm::vec3 fragPos(worldPos);

    glm::vec3 lightDir = glm::normalize(lightPos - fragPos);
    glm::vec3 viewDir = glm::normalize(viewPos - fragPos);

    glm::vec3 h = lightDir + viewDir;
#if 0
    // // 处理 h 向量为零的情况
    if (glm::length(h) < 1e-8f){
        //h = normal; // 或直接跳过 spec
        std::cout << "========" << std::endl;
    }
#endif
    glm::vec3 halfwayDir = glm::normalize(h);

    // 环境光分量
    float ambientStrength = 0.4f;
    glm::vec3 ambient = ambientStrength * lightAmbient * lightColor * objectColor;

    if (0)
    {
        // 2. 仅环境光
        gl_FragColor = glm::vec4(ambient, 1.0f);
        return;
    }

    // 漫反射分量 (Lambert)
    float diff = glm::max(glm::dot(normal, lightDir), 0.0f);
    glm::vec3 diffuse = diff * lightDiffuse * lightColor * objectColor;

    if (0)
    {
        // 3. 仅漫反射光
        gl_FragColor = glm::vec4(diffuse, 1.0f);
        return;
    }

    //控制高光亮斑的大小
    float shininess = 8; // 橡皮/布料质感
    //shininess = 32; //塑料质感（当前）
    //shininess = 64; // 硬塑料
    //shininess = 128; // 金属/陶瓷

    //shininess = 1.f;
    //shininess = 2.f;
    //shininess = 512.f;
    shininess = 64;

    // 镜面反射分量 (Blinn-Phong)
    //float spec = glm::pow(glm::max(glm::dot(normal, halfwayDir), 0.0f), shininess);
    float spec = glm::pow(glm::clamp(glm::dot(normal, halfwayDir), 0.0f, 1.0f), shininess);
    glm::vec3 specular = spec * lightSpecular * lightColor;

    if (0)
    {
        // 4. 仅镜面反射光（通常为白色高光，不乘物体色）
        gl_FragColor = glm::vec4(specular, 1.0f);
        return;
    }

    // 阴影（平行光正交 + 硬阴影 + 固定 bias，逻辑见 ShadowHelper.h）
    float shadow = computeShadowFactor(*this, fragPos);

    if (1)
    {
        // 5. 组合光照：环境光不受阴影影响
        //glm::vec3 result = ambient + diffuse + specular;
        glm::vec3 result = ambient + shadow * diffuse;// + specular;
        gl_FragColor = glm::vec4(result, 1.0f);
        return;
    }
}

#endif
