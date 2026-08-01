#ifndef RENDER_HELPER_H
#define RENDER_HELPER_H

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/geometric.hpp>

#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.1415926
#endif

inline double degrees_to_radians(double degrees)
{
    return degrees * (M_PI / 180.0);
}

inline double radians_to_degrees(double radians)
{
    return radians * (180.0 / M_PI);
}

inline float calculateFrontFace2D(glm::vec2 v0, glm::vec2 v1, glm::vec2 v2)
{
    auto n = v1 - v0;
    auto m = v2 - v0;
    return n.x * m.y - n.y * m.x;
}

// 边界函数法​​（vec2 叉乘计算的是平行四边形的面积）
inline float edgeFunc(glm::vec2 a, glm::vec2 b, float x, float y)
{
    return (b.x - a.x) * (y - a.y) - (b.y - a.y) * (x - a.x);
}

// 重心坐标法​
inline glm::vec3 barycentric(glm::vec2 v0, glm::vec2 v1, glm::vec2 v2, float x, float y)
{
    float area = edgeFunc(v0, v1, v2.x, v2.y);
    float w0 = edgeFunc(v1, v2, x, y) / area;
    float w1 = edgeFunc(v2, v0, x, y) / area;
    float w2 = 1 - w0 - w1;
    return glm::vec3(w0, w1, w2);
}

// 等价于barycentric
inline glm::vec3 calculateBarycentric(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 p)
{
    // 计算三角形面积（使用叉积）
    glm::vec2 ab = b - a;
    glm::vec2 ac = c - a;
    glm::vec2 ap = p - a;

    float area_abc = ab.x * ac.y - ab.y * ac.x; // 2倍三角形面积

    // if (std::abs(area_abc) < 1e-6) {
    //     // 三角形面积为零，返回无效的重心坐标
    //     return glm::vec3(-1.0f, -1.0f, -1.0f);
    // }

    // 计算子三角形面积
    glm::vec2 bp = p - b;
    glm::vec2 cp = p - c;

    // PBC三角形的面积（2倍）
    float area_pbc = (b.x - p.x) * (c.y - p.y) - (b.y - p.y) * (c.x - p.x);
    // PCA三角形的面积（2倍）
    float area_pca = (c.x - p.x) * (a.y - p.y) - (c.y - p.y) * (a.x - p.x);

    // 计算重心坐标
    float alpha = area_pbc / area_abc;
    float beta = area_pca / area_abc;
    float gamma = 1.0f - alpha - beta;

    return glm::vec3(alpha, beta, gamma);
}

inline void calculateBoundingBox(const glm::vec2 &v0, const glm::vec2 &v1, const glm::vec2 &v2,
                          int &minX, int &maxX, int &minY, int &maxY,
                          int width, int height)
{
    // 获取最小/最大X值
    float fminX = std::min({v0.x, v1.x, v2.x});
    float fmaxX = std::max({v0.x, v1.x, v2.x});

    // 获取最小/最大Y值
    float fminY = std::min({v0.y, v1.y, v2.y});
    float fmaxY = std::max({v0.y, v1.y, v2.y});

    // 转换为整数并夹紧到屏幕范围
    minX = static_cast<int>(std::floor(fminX));
    maxX = static_cast<int>(std::ceil(fmaxX));
    minY = static_cast<int>(std::floor(fminY));
    maxY = static_cast<int>(std::ceil(fmaxY));

    // 确保在屏幕范围内
    minX = std::max(0, minX);
    maxX = std::min(width - 1, maxX);
    minY = std::max(0, minY);
    maxY = std::min(height - 1, maxY);
}

// ============================================================
// 射线-三角形相交（Möller-Trumbore 算法）
// ============================================================

struct Ray
{
    glm::vec3 origin;
    glm::vec3 direction;
};

// Möller–Trumbore算法 https://zhuanlan.zhihu.com/p/19176879229
// 给定一条射线 R(t)=O+td 和一个三角形（顶点为V0​、V1​、V2​），判断射线是否与三角形相交。如果相交，返回交点的位置t 
/// 返回 true 如果射线与三角形相交
/// orig     射线起点
/// dir      射线方向（需归一化）
/// v0,v1,v2 三角形三个顶点
/// t        输出：交点距离 orig + t * dir
inline bool rayTriangleIntersect(const glm::vec3 &orig, const glm::vec3 &dir,
                                 const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2,
                                 float &t)
{
    constexpr float EPSILON = 1e-6f;
    glm::vec3 e1 = v1 - v0;
    glm::vec3 e2 = v2 - v0;
    glm::vec3 p = glm::cross(dir, e2);
    float det = glm::dot(e1, p);

    if (std::fabs(det) < EPSILON)
        return false; // 射线平行于三角形

    float invDet = 1.0f / det;
    glm::vec3 s = orig - v0;
    float u = glm::dot(s, p) * invDet;
    if (u < 0.0f || u > 1.0f)
        return false;

    glm::vec3 q = glm::cross(s, e1);
    float v = glm::dot(dir, q) * invDet;
    if (v < 0.0f || u + v > 1.0f)
        return false;

    t = glm::dot(e2, q) * invDet;
    return t > 0.0f;
}

#endif // RENDER_HELPER_H