#ifndef SHADOW_HELPER_H
#define SHADOW_HELPER_H

#include "Shader.h"
#include <glm/glm.hpp>

// 阴影因子：1 = 被光照亮，0 = 在阴影中（平行光正交 + 硬阴影 + 固定 bias）
// 供各片元着色器复用，要求着色器已具备 shadowEnabled / lightViewProjectionMatrix / shadowBias
// 与阴影贴图纹理（SHADOW_MAP_TEXTURE_UNIT）
inline float computeShadowFactor(Shader &shader, const glm::vec3 &fragPos)
{
    if (shader.getUniform<float>("shadowEnabled") <= 0.5f)
        return 1.0f;

    glm::mat4 lightVP = shader.getUniform<glm::mat4>("lightViewProjectionMatrix");
    glm::vec4 lightClip = lightVP * glm::vec4(fragPos, 1.0f);
    glm::vec3 ndc = glm::vec3(lightClip) / lightClip.w;

    // 光源裁剪空间 → 阴影贴图 UV（y 翻转与深度 pass 一致）
    glm::vec2 shadowUV(ndc.x * 0.5f + 0.5f, 0.5f - ndc.y * 0.5f);
    float currentDepth = ndc.z * 0.5f + 0.5f;
    float bias = shader.getUniform<float>("shadowBias");
    float closestDepth = shader.getTexture(SHADOW_MAP_TEXTURE_UNIT)->sample(shadowUV).r;

    return (currentDepth - bias > closestDepth) ? 0.0f : 1.0f;
}

#endif // SHADOW_HELPER_H
