#include "Render.h"

#include <array>
#include <iostream>
#include <algorithm>
#include <limits>
#include <chrono>

#include <cmath>
#include <glm/glm.hpp>

#include "ColorBlend.h"
#include "TriangleData.h"
#include "RenderHelper.h"

Render::Render(int width, int height)
    : _width(width), _height(height), _frameBuffer(width * height * 4, 255), _depthBuffer(width * height, std::numeric_limits<float>::max())
{
    _camera = std::make_shared<Camera>();
}

Render::~Render()
{
}

void Render::initVertexArray(std::vector<float> &&vertexArray, std::vector<unsigned int> vertexIndexArray)
{
    _vertexArray = std::move(vertexArray);
    _vertexIndexArray = std::move(vertexIndexArray);
}

void Render::setTexture(std::shared_ptr<Texture> texture)
{
    _texture = texture;
}

void Render::setLayout(int vertexLayout, int vertexSize, int colorLayout, int colorSize, int uvLayout, int uvSize)
{
    _vertexLayout = vertexLayout;
    _vertexSize = vertexSize;
    _colorLayout = colorLayout;
    _colorSize = colorSize;
    _uvLayout = uvLayout;
    _uvSize = uvSize;
}

void Render::clear()
{
    std::fill(_frameBuffer.begin(), _frameBuffer.end(), 255);
    std::fill(_depthBuffer.begin(), _depthBuffer.end(), std::numeric_limits<float>::max());
}

void Render::frame()
{
    // 清理颜色和深度缓冲区
    clear();
  
    auto frameStartTime = std::chrono::steady_clock::now();

    // 顶点着色器输出顶点
    std::vector<glm::vec4> vertexArrayDeal{};

    // 顶点着色器输出颜色
    std::vector<glm::vec4> colorArrayDeal{};

    // 顶点着色器输出uv
    std::vector<glm::vec2> uvArrayDeal{};

    // 构建矩阵
    auto modelMatrix = glm::mat4x4(1.0);

    // 相机回调
    _camera->update(_lastFrameTime, _renderTime, _renderCount);

    // 创建视图矩阵
    glm::vec3 eye, center, up;
    glm::mat4 viewMatrix = _camera->getViewMatrix(eye, center, up);

    // 创建透视投影矩阵
    float fov, aspect, near, far;
    glm::mat4 projectionMatrix = _camera->getProjectionMatrix(fov, aspect, near, far);

    glm::mat4 mpv = projectionMatrix * viewMatrix * modelMatrix;

    // MSAA多重采样
    constexpr int msaaSampleCount = 4; // 4x4 多重采样
    // 采样偏移(这是一个旋转的网格采样（Rotated grid），它并不是对称的，同样还有其他几个采样方式：网格(Grid)，网格采样时对称的，但是效果不好、随机（Random）、抖动（Jittered）)
    const std::array<glm::vec2, msaaSampleCount> sampleOffsets = {
        glm::vec2{0.375f, 0.125f},
        glm::vec2{0.125f, 0.375f},
        glm::vec2{0.625f, 0.875f},
        glm::vec2{0.875f, 0.625f}};

    // MSAA缓冲区，记录每个像素的4个采样点的深度值和颜色
    auto maxFloat = std::numeric_limits<float>::max();
    std::vector<std::array<float, msaaSampleCount>> msaaDepthBuffer(_width * _height,
                                                                    {maxFloat, maxFloat, maxFloat, maxFloat});

    std::vector<std::array<glm::vec4, msaaSampleCount>> msaaColorBuffer(_width * _height,
                                                                        {glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f)});

    // 处理顶点属性
    auto stride = _vertexSize + _colorSize + _uvSize;
    for (size_t i = 0; i < _vertexArray.size(); i += stride)
    {
        auto vert = glm::vec3(_vertexArray[i], _vertexArray[i + 1], _vertexArray[i + 2]);
        vertexArrayDeal.emplace_back(mpv * glm::vec4(vert, 1.0f));

        auto color = glm::vec4(_vertexArray[i + 3], _vertexArray[i + 4], _vertexArray[i + 5], _vertexArray[i + 6]);
        colorArrayDeal.emplace_back(color);

        if (_uvSize != 0)
        {
            auto uv = glm::vec2(_vertexArray[i + 7], _vertexArray[i + 8]);
            uvArrayDeal.emplace_back(uv);
        }

        // auto normal = glm::vec3(_vertexArray[i + 9], _vertexArray[i + 10], _vertexArray[i + 11]);
        // normalArrayDeal.emplace_back(normal);
    }

    // 图元处理
    for (size_t i = 0; i < _vertexIndexArray.size(); i += 3)
    {
        Triangle tri(
            vertexArrayDeal[_vertexIndexArray[i]],
            vertexArrayDeal[_vertexIndexArray[i + 1]],
            vertexArrayDeal[_vertexIndexArray[i + 2]]);

        if (!colorArrayDeal.empty())
        {
            tri._hasColor = true;
            tri._color[0] = colorArrayDeal[_vertexIndexArray[i]];
            tri._color[1] = colorArrayDeal[_vertexIndexArray[i + 1]];
            tri._color[2] = colorArrayDeal[_vertexIndexArray[i + 2]];
        }

        if (!uvArrayDeal.empty())
        {
            tri._hasUV = true;
            tri._uv[0] = uvArrayDeal[_vertexIndexArray[i]];
            tri._uv[1] = uvArrayDeal[_vertexIndexArray[i + 1]];
            tri._uv[2] = uvArrayDeal[_vertexIndexArray[i + 2]];
        }

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

        // 正面/背面剔除 (立方体建模可能有问题，在立方体旋转时会有面确实)
        if (0)
        {
            auto front = calculateFrontFace2D(tri._screen_position[0], tri._screen_position[1], tri._screen_position[2]);

            // 开启背面剔除
            if (1 && front < 0)
            {
                continue;
            }

            // 开启正面面剔除
            if (0 && front > 0)
            {
                continue;
            }
        }

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
                // 记录当前像素索引
                int pixelIndex = y * _width + x;
                bool isCovered = false;
                std::array<bool, msaaSampleCount> sampleCoveredState = {false, false, false, false};

                for (int sampleIndex = 0; sampleIndex < msaaSampleCount; sampleIndex++)
                {
                    // 计算采样中心
                    float x_sample = x + sampleOffsets[sampleIndex].x;
                    float y_sample = y + sampleOffsets[sampleIndex].y;

                    // 计算三角形重心坐标(α，β，γ)，决定了每个三个顶点对该像素或采样的影响因子
                    glm::vec3 weights = barycentric(tri._screen_position[0], tri._screen_position[1], tri._screen_position[2], x_sample, y_sample);

                    // 检查像素或采样是否在三角形内
                    if (weights.x < 0 || weights.y < 0 || weights.z < 0)
                        continue;

                    /**
                     * 插值算法
                    float one_over_w = α * (1.0 / w0) + β * (1.0 / w1) + γ * (1.0 / w2);
                    float interpolated_w = 1.0 / one_over_w;

                    uv = (α * (uv0 / w0) + β * (uv1 / w1) + γ * (uv2 / w2)) * interpolated_w;
                    depth = (α * (depth0 / w0) + β * (depth1 / w1) + γ * (depth2 / w2)) * interpolated_w;
                    color = (α * (color0 / w0) + β * (color1 / w1) + γ * (color2 / w2)) * interpolated_w;
                    */

                    // 透视校正插值，使用NDC坐标
                    float interpolated_w = 1.0 / (weights.x * w0_inv + weights.y * w1_inv + weights.z * w2_inv);
                    float z_ndc = (weights.x * (tri._ndc_position[0].z * w0_inv) +
                                   weights.y * (tri._ndc_position[1].z * w1_inv) +
                                   weights.z * (tri._ndc_position[2].z * w2_inv)) *
                                  interpolated_w;

                    // 转换为深度缓冲值(depthMax,depthMin可定义，znear,zfar和是-1到1)
                    float depth = z_ndc * 0.5f + 0.5f; //(z_ndc - znear) / (zfar - znear) * (depthMax - depthMin) + depthMin;

                    // 开启深度测试，记录深度测试通过的采样点
                    if (1 && depth > msaaDepthBuffer[pixelIndex][sampleIndex])
                        continue;

                    isCovered = true;
                    sampleCoveredState[sampleIndex] = true;

                    // 写入深度缓冲区
                    if (1)
                    {
                        msaaDepthBuffer[pixelIndex][sampleIndex] = depth;
                    }
                }

                // 调用片段着色器
                if (isCovered)
                {
                    // msaa只插值中心像素点的颜色，然后将颜色复制给被三角形覆盖后的采样点
                    glm::vec3 weights = barycentric(tri._screen_position[0], tri._screen_position[1], tri._screen_position[2], x + 0.5, y + 0.5);
                    float interpolated_w = 1.0 / (weights.x * w0_inv + weights.y * w1_inv + weights.z * w2_inv);

                    glm::vec4 color = glm::vec4(0.0f);
                    if (0)
                    {
                        // 平坦着色,逐顶点着色 取一个三角形任意顶点的颜色（opengl默认取三角形最后一个顶点，但是可以修改）
                        // color = tri._color[0] + tri._color[1] + tri._color[2];
                        // color /= 3;
                        color = tri._color[2];
                    }
                    else // 逐像素着色
                    {
                        if (tri._hasColor)
                        {
                            color =
                                (weights.x * (tri._color[0] * w0_inv) +
                                 weights.y * (tri._color[1] * w1_inv) +
                                 weights.z * (tri._color[2] * w2_inv)) *
                                interpolated_w;
                        }

                        if (tri._hasUV && _texture)
                        {
                            glm::vec2 uv =
                                (weights.x * (tri._uv[0] * w0_inv) +
                                 weights.y * (tri._uv[1] * w1_inv) +
                                 weights.z * (tri._uv[2] * w2_inv)) *
                                interpolated_w;

                            auto uvcolor = _texture->sampleBilinear(uv.x, uv.y);

                            color = customBlend(uvcolor, color, 0.4, 0.6);
                        }
                    }

                    // 写入颜色缓冲区，只更新深度测试通过的采样点
                    for (int sampleIndex = 0; sampleIndex < msaaSampleCount; sampleIndex++)
                    {
                        if (sampleCoveredState[sampleIndex])
                        {
                            // 开启颜色混合
                            auto oldColor = msaaColorBuffer[pixelIndex][sampleIndex];
                            msaaColorBuffer[pixelIndex][sampleIndex] = alphaBlend(color, oldColor);
                        }
                    }
                }
            }
        }
    }

    // msaa采样解析
    // 写入帧缓冲区
    int pixelIndex = 0;
    for (auto it = msaaColorBuffer.begin(); it != msaaColorBuffer.end(); it++)
    {
        std::array<glm::vec4, msaaSampleCount> &colorArray = *it;

        glm::vec4 color = glm::vec4(0.0f);
        for (int i = 0; i < msaaSampleCount; i++)
        {
            color += colorArray[i];
        }
        color /= msaaSampleCount;

        _frameBuffer[pixelIndex * 4] = color.r * 255;
        _frameBuffer[pixelIndex * 4 + 1] = color.g * 255;
        _frameBuffer[pixelIndex * 4 + 2] = color.b * 255;
        _frameBuffer[pixelIndex * 4 + 3] = color.a * 255;

        pixelIndex++;
    }

    auto frameEndTime = std::chrono::steady_clock::now();
    _lastFrameTime = (std::chrono::duration<float>(frameEndTime - frameStartTime)).count();
    _renderTime += _lastFrameTime;
    _renderCount++;
}
