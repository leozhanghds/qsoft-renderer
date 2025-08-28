#ifndef COLOR_BLEND_H
#define COLOR_BLEND_H

#pragma once

#include <glm/glm.hpp>

// 混合因子枚举，类似于 OpenGL 的混合因子
enum BlendFactor {
    ZERO,
    ONE,
    SRC_COLOR,
    ONE_MINUS_SRC_COLOR,
    DST_COLOR,
    ONE_MINUS_DST_COLOR,
    SRC_ALPHA,
    ONE_MINUS_SRC_ALPHA,
    DST_ALPHA,
    ONE_MINUS_DST_ALPHA
};

// 混合函数，类似于 OpenGL 的 glBlendFunc
inline glm::vec4 blend(glm::vec4 src, glm::vec4 dst, 
                BlendFactor srcFactor, BlendFactor dstFactor) {
    
    // 计算源因子
    glm::vec4 srcFactorValue;
    switch (srcFactor) {
        case ZERO: srcFactorValue = glm::vec4(0.0f); break;
        case ONE: srcFactorValue = glm::vec4(1.0f); break;
        case SRC_COLOR: srcFactorValue = src; break;
        case ONE_MINUS_SRC_COLOR: srcFactorValue = glm::vec4(1.0f) - src; break;
        case DST_COLOR: srcFactorValue = dst; break;
        case ONE_MINUS_DST_COLOR: srcFactorValue = glm::vec4(1.0f) - dst; break;
        case SRC_ALPHA: srcFactorValue = glm::vec4(src.a); break;
        case ONE_MINUS_SRC_ALPHA: srcFactorValue = glm::vec4(1.0f - src.a); break;
        case DST_ALPHA: srcFactorValue = glm::vec4(dst.a); break;
        case ONE_MINUS_DST_ALPHA: srcFactorValue = glm::vec4(1.0f - dst.a); break;
        default: srcFactorValue = glm::vec4(1.0f); break;
    }
    
    // 计算目标因子
    glm::vec4 dstFactorValue;
    switch (dstFactor) {
        case ZERO: dstFactorValue = glm::vec4(0.0f); break;
        case ONE: dstFactorValue = glm::vec4(1.0f); break;
        case SRC_COLOR: dstFactorValue = src; break;
        case ONE_MINUS_SRC_COLOR: dstFactorValue = glm::vec4(1.0f) - src; break;
        case DST_COLOR: dstFactorValue = dst; break;
        case ONE_MINUS_DST_COLOR: dstFactorValue = glm::vec4(1.0f) - dst; break;
        case SRC_ALPHA: dstFactorValue = glm::vec4(src.a); break;
        case ONE_MINUS_SRC_ALPHA: dstFactorValue = glm::vec4(1.0f - src.a); break;
        case DST_ALPHA: dstFactorValue = glm::vec4(dst.a); break;
        case ONE_MINUS_DST_ALPHA: dstFactorValue = glm::vec4(1.0f - dst.a); break;
        default: dstFactorValue = glm::vec4(1.0f); break;
    }
    
    // 应用混合公式: result = src * srcFactor + dst * dstFactor
    return src * srcFactorValue + dst * dstFactorValue;
}

// 简化的混合函数，使用最常见的混合模式
inline glm::vec4 alphaBlend(glm::vec4 src, glm::vec4 dst) {
    return blend(src, dst, SRC_ALPHA, ONE_MINUS_SRC_ALPHA);
}

// 加法混合
inline glm::vec4 additiveBlend(glm::vec4 src, glm::vec4 dst) {
    return blend(src, dst, SRC_ALPHA, ONE);
}

// 乘法混合
inline glm::vec4 multiplyBlend(glm::vec4 src, glm::vec4 dst) {
    return blend(src, dst, DST_COLOR, ZERO);
}

// 自定义比例混合函数
inline glm::vec4 customBlend(glm::vec4 src, glm::vec4 dst, float srcWeight, float dstWeight) {
    return src * glm::vec4(srcWeight) + dst * glm::vec4(dstWeight);
}


#endif // COLOR_BLEND_H
