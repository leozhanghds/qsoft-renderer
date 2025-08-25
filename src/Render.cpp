#include "Render.h"

#include <iostream>
#include <algorithm>
#include <limits>
#include <chrono>

#include <cmath>
#include <glm.hpp>
#include <ext/matrix_transform.hpp>
#include <ext/matrix_clip_space.hpp>

#define M_PI 3.1415926

double degrees_to_radians(double degrees)
{
    return degrees * (M_PI / 180.0);
}

double radians_to_degrees(double radians)
{
    return radians * (180.0 / M_PI);
}

Render::Render(int width, int height)
    : _width(width), _height(height), _frameBuffer(width * height * 4, 255), _depthBuffer(width * height, std::numeric_limits<float>::max())
{
    _startTime = std::chrono::steady_clock::now();
}

Render::~Render()
{
}

void Render::initVertexArray(std::vector<float> &&vertexArray, std::vector<unsigned int> vertexIndexArray)
{
    _vertexArray = std::move(vertexArray);
    _vertexIndexArray = std::move(vertexIndexArray);
}

void Render::clear()
{
    std::fill(_frameBuffer.begin(), _frameBuffer.end(), 255);
    std::fill(_depthBuffer.begin(), _depthBuffer.end(), std::numeric_limits<float>::max());
}

double d = 0;

void Render::testRender()
{
    for (int i = 0; i < _width * _height * 4; i += 4)
    {
        auto v = std::sin(degrees_to_radians(d));
        _frameBuffer[i] = 255 * std::abs(d);

        _frameBuffer[i + 1] = 0;
        _frameBuffer[i + 2] = 0;
        _frameBuffer[i + 3] = 255;
    }

    d += 0.1;
}

// 每个顶点个数大小
int vertexSize = 3;
// 每个顶点个数大小
int colorSize = 4;

// 顶点着色器输出顶点
std::vector<glm::vec4> vertexArrayDeal{};

// 顶点着色器输出颜色
std::vector<glm::vec4> colorArrayDeal{};

class Triangle
{
public:
    Triangle(glm::vec4 v1, glm::vec4 v2, glm::vec4 v3)
    {
        _position[0] = v1;
        _position[1] = v2;
        _position[2] = v3;
    }
    // vertex
    glm::vec4 _position[3];

    // color
    glm::vec4 _color[3]{};

    // texture
    glm::vec2 _uv[3]{};

    // ndc_position
    glm::vec3 _ndc_position[3]{};

    // screen_position
    glm::vec2 _screen_position[3]{};
};

// 使用OpenGL默认深度范围 [0, 1]
const float depth_min = 0.0f;
const float depth_max = 1.0f;

// 边界函数法​​（叉乘计算面积）
float edgeFunc(glm::vec2 a, glm::vec2 b, int x, int y)
{
    return (b.x - a.x) * (y - a.y) - (b.y - a.y) * (x - a.x);
}

// 重心坐标法​
glm::vec3 barycentric(glm::vec2 v0, glm::vec2 v1, glm::vec2 v2, int x, int y)
{
    float area = edgeFunc(v0, v1, v2.x, v2.y);
    float w0 = edgeFunc(v1, v2, x, y) / area;
    float w1 = edgeFunc(v2, v0, x, y) / area;
    float w2 = 1 - w0 - w1;
    return glm::vec3(w0, w1, w2);
}

// 等价于barycentric
glm::vec3 calculateBarycentric(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 p)
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

void calculateBoundingBox(const glm::vec2 &v0, const glm::vec2 &v1, const glm::vec2 &v2,
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

void Render::frame()
{
    clear();

    vertexArrayDeal.clear();
    colorArrayDeal.clear();

    std::chrono::duration<double> durationSeconds = std::chrono::steady_clock::now() - _startTime;

    //std::cout << "durationSeconds: " << durationSeconds.count() << std::endl;

    // 构建矩阵
    auto modelMatrix = glm::mat4x4(1.0);

    glm::vec3 eye(4.0f, 3.0f, 3.0f); // 看向-z方向
    //glm::vec3 eye(2.0f, 2.0f, 10.0f); // 看向-z方向
    eye = glm::vec3(0.0f, 0.0f, 10.0f); // 看向-z方向
    if(1){
        float radius = 10.0f;
        float camX = sin(durationSeconds.count()) * radius;
        float camZ = cos(durationSeconds.count()) * radius;
        //float camZ = 0.f;
        eye = glm::vec3(camX, 0.0f, camZ); // 周期运动
    }
    
    glm::vec3 center(0.0f, 0.0f, 0.0f);
    glm::vec3 up(0.0f, 1.0f, 0.0f);

    // 创建视图矩阵
    glm::mat4 viewMatrix = glm::lookAt(eye, center, up);

    float aspectRatio = 800.0f / 600.0f;
    float fovy = 45.0f; // z y 与viewDir的夹角
    float nearPlane = 1.0f;
    float farPlane = 100.0f;

    // 创建透视投影矩阵
    glm::mat4 projectionMatrix = glm::perspective(fovy, aspectRatio, nearPlane, farPlane);

    glm::mat4 mpv = projectionMatrix * viewMatrix * modelMatrix;

    // 处理顶点属性
    for (size_t i = 0; i < _vertexArray.size(); i += (vertexSize + colorSize))
    {
        auto vert = glm::vec3(_vertexArray[i], _vertexArray[i + 1], _vertexArray[i + 2]);
        vertexArrayDeal.emplace_back(mpv * glm::vec4(vert, 1.0f));

        auto color = glm::vec4(_vertexArray[i + 3], _vertexArray[i + 4], _vertexArray[i + 5], _vertexArray[i + 6]);
        colorArrayDeal.emplace_back(color);
    }

    // std::cout << "deal vertex arrtibute" << std::endl;

    // 图元处理
    int count = 0;
    for (size_t i = 0; i < _vertexIndexArray.size(); i += 3)
    {
        //std::cout << "Triangle Count: " << ++count << std::endl;

        Triangle tri(vertexArrayDeal[_vertexIndexArray[i]], vertexArrayDeal[_vertexIndexArray[i + 1]], vertexArrayDeal[_vertexIndexArray[i + 2]]);

        tri._color[0] = colorArrayDeal[i];
        tri._color[1] = colorArrayDeal[i + 1];
        tri._color[2] = colorArrayDeal[i + 2];

        // 裁剪视锥体(先省略)

        // 透视除法  在 NDC 中，坐标范围被归一化到([-1, 1])的标准区间
        tri._ndc_position[0] = tri._position[0] / tri._position[0].w;
        tri._ndc_position[1] = tri._position[1] / tri._position[1].w;
        tri._ndc_position[2] = tri._position[2] / tri._position[2].w;

        // 视口变换  将 NDC 坐标映射到屏幕坐标 屏幕坐标的范围是[0, width-1]和[0, height-1]
        // 注意：y 坐标需要取反，因为 OpenGL 中的 y 轴是从下到上的
        tri._screen_position[0].x = (tri._ndc_position[0].x + 1) * (_width - 1) / 2;
        tri._screen_position[0].y = (-tri._ndc_position[0].y + 1) * (_height - 1) / 2;

        tri._screen_position[1].x = (tri._ndc_position[1].x + 1) * (_width - 1) / 2;
        tri._screen_position[1].y = (-tri._ndc_position[1].y + 1) * (_height - 1) / 2;

        tri._screen_position[2].x = (tri._ndc_position[2].x + 1) * (_width - 1) / 2;
        tri._screen_position[2].y = (-tri._ndc_position[2].y + 1) * (_height - 1) / 2;

        // 背面剔除（先省略）
        // if (isBackFace(tri)) continue;

        // 光栅化（生成覆盖的像素片段）
        // 计算包围盒
        int minX, maxX, minY, maxY;
        calculateBoundingBox(tri._screen_position[0], tri._screen_position[1], tri._screen_position[2], minX, maxX, minY, maxY, _width, _height);

        // 预计算透视校正所需的因子
        float w0_inv = 1.0f / tri._position[0].w;
        float w1_inv = 1.0f / tri._position[1].w;
        float w2_inv = 1.0f / tri._position[2].w;

        // 遍历包围盒中的像素
        for (int y = minY; y <= maxY; y++)
        {
            for (int x = minX; x <= maxX; x++)
            {
                // 计算三角形重心坐标(α，β，γ)，决定了每个三个顶点对该像素的影响因子
                // 像素中心​​：位于 (x + 0.5, y + 0.5)的点
                glm::vec3 weights = barycentric(tri._screen_position[0], tri._screen_position[1], tri._screen_position[2], x + 0.5f, y + 0.5f);

                // 检查像素是否在三角形内
                if (weights.x < 0 || weights.y < 0 || weights.z < 0)
                    continue;

                /**
                float one_over_w = α * (1.0 / w0) + β * (1.0 / w1) + γ * (1.0 / w2);
                float interpolated_w = 1.0 / one_over_w;

                uv = (α * (uv0 / w0) + β * (uv1 / w1) + γ * (uv2 / w2)) * interpolated_w;
                depth = (α * (depth0 / w0) + β * (depth1 / w1) + γ * (depth2 / w2)) * interpolated_w;
                color = (α * (color0 / w0) + β * (color1 / w1) + γ * (color2 / w2)) * interpolated_w;
                */

                // 透视校正插值，使用裁剪坐标
                float interpolated_w = 1.0 / (weights.x * w0_inv + weights.y * w1_inv + weights.z * w2_inv);
                float z_ndc = (weights.x * (tri._ndc_position[0].z * w0_inv) +
                               weights.y * (tri._ndc_position[1].z * w1_inv) +
                               weights.z * (tri._ndc_position[2].z * w2_inv)) *
                              interpolated_w;

                // 转换为深度缓冲值(depthMax,depthMin可定义，znear,zfar和是-1到1)
                float depth = z_ndc * 0.5f + 0.5f; //(z_ndc - znear) / (zfar - znear) * (depthMax - depthMin) + depthMin;

                int pixelIndex = y * _width + x;

                // 深度测试
                if (depth > _depthBuffer[pixelIndex])
                    continue;

                // 调用片段着色器
                // Color color = fragmentShader(uv);
                glm::vec4 color =
                    (weights.x * (tri._color[0] * w0_inv) +
                     weights.y * (tri._color[1] * w1_inv) +
                     weights.z * (tri._color[2] * w2_inv)) *
                    interpolated_w;

                // 写入帧缓冲区
                _depthBuffer[pixelIndex] = depth;

                _frameBuffer[pixelIndex * 4] = color.r * 255;
                _frameBuffer[pixelIndex * 4 + 1] = color.g * 255;
                _frameBuffer[pixelIndex * 4 + 2] = color.b * 255;
                _frameBuffer[pixelIndex * 4 + 3] = color.a * 255;
            }
        }
    }
}
