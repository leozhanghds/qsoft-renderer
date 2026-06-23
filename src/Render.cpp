#include "Render.h"

#include <array>
#include <iostream>
#include <algorithm>
#include <limits>
#include <chrono>
#include <unordered_map>

#include <cmath>
#include <glm/glm.hpp>

#include "ColorBlend.h"
#include "TriangleData.h"
#include "RenderHelper.h"

#include <QImage>

Render::Render(std::unique_ptr<DoubleBuffer> &buffer, int width, int height)
    : _width(width), _height(height), _doubleBuffer(buffer),
      _msaaDepthBuffer(width * height, {FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX}),
      _msaaColorBuffer(width * height, {glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f)}),
      _msaaStencilBuffer(width * height, {0, 0, 0, 0})
{
    _camera = std::make_shared<Camera>(width, height);
    _doubleBuffer->back().resize(width, height);
}

Render::~Render()
{
}

void Render::submitCommand(RenderCommand cmd)
{
    std::lock_guard<std::mutex> lock(_cmdMutex);
    _commands.push(std::move(cmd));
}

void Render::addNode(std::shared_ptr<Node> node)
{
    _nodes.emplace_back(node);
}

void Render::removeNode(std::shared_ptr<Node> node)
{
    auto it = std::find(_nodes.begin(), _nodes.end(), node);
    if (it != _nodes.end())
    {
        _nodes.erase(it);
    }
}
void Render::resize(int width, int height)
{
    if (_width != width || _height != height)
    {
        _width = width;
        _height = height;
        _msaaDepthBuffer.resize(width * height, {FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX});
        _msaaColorBuffer.resize(width * height, {glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f)});
        _msaaStencilBuffer.resize(width * height, {0, 0, 0, 0});

        float fov, near, far;
        float aspect = (float)_width / (float)_height;
        _camera->getProjectionMatrix(fov, aspect, near, far);
        _camera->setProjectionMatrix(fov, aspect, near, far);
    }
}

void Render::processCommands()
{
    std::queue<RenderCommand> local;
    {
        std::lock_guard<std::mutex> lock(_cmdMutex);
        std::swap(local, _commands);
    }

    while (!local.empty())
    {
        RenderCommand cmd = std::move(local.front());
        local.pop();

        switch (cmd.type)
        {
        case RenderCommand::Type::AddNode:
            addNode(cmd.node);
            break;
        case RenderCommand::Type::RemoveNode:
            removeNode(cmd.node);
            break;
        case RenderCommand::Type::Resize:
            resize(cmd.pixelWidth, cmd.pixelHeight);
            break;
        default:
            break;
        }
    }
}

void Render::renderOneFrame()
{
    // 只在渲染线程调用
    processCommands();

    // 对齐缓冲区大小
    FrameBuffer &frameBuffer = _doubleBuffer->back();
    if (_width != frameBuffer.width || _height != frameBuffer.height)
    {
        frameBuffer.resize(_width, _height);
    }

    clearBuffer(frameBuffer, CLEAR_COLOR_BUFFER | CLEAR_DEPTH_BUFFER);
    drawScene(frameBuffer);

    _doubleBuffer->swap();
}

void Render::clearBuffer(FrameBuffer &frameBuffer, std::bitset<4> clearFlags)
{
    if (clearFlags.test(0))
    {
        std::fill_n(frameBuffer.pixels.data(), frameBuffer.pixels.size(), 255);
    }

    if (clearFlags.test(1))
    {
        // 当成一个大的数组进行快读填充
        std::fill_n(_msaaDepthBuffer.data()->data(), _msaaDepthBuffer.size() * MSAA_SAMPLE_COUNT, FLT_MAX);
        std::fill_n(_msaaColorBuffer.data()->data(), _msaaColorBuffer.size() * MSAA_SAMPLE_COUNT, glm::vec4(0.0f));
    }

    if (clearFlags.test(2))
    {
        std::fill_n(_msaaStencilBuffer.data()->data(), _msaaStencilBuffer.size() * MSAA_SAMPLE_COUNT, 0);
    }
}

void Render::drawScene(FrameBuffer &frameBuffer)
{
    static auto firstFrameTime = std::chrono::steady_clock::now();

    auto frameStartTime = std::chrono::steady_clock::now();

    // 顶点着色器输出顶点属性
    std::vector<std::vector<float>> vertexShaderOutArray{};
    // 顶点着色器输出顶点
    std::vector<glm::vec4> gl_PositionArray{};
    {
        // 预先分配顶点着色器输出内存
        size_t vertexArraySize = 0;
        for (auto &node : _nodes)
        {
            vertexArraySize += node->getVertexArray().size() / node->getStride();
        }
        vertexShaderOutArray.resize(vertexArraySize, std::vector<float>(MAX_VERTEX_OUTPUT_MEMORY_SIZE));

        gl_PositionArray.resize(vertexArraySize, glm::vec4(0.0f));
    }

    // 相机回调
    _camera->update(_lastFrameTime, _renderTime, _renderCount);

    // 构建矩阵
    auto modelMatrix = glm::mat4x4(1.0);

    // 获取视图矩阵
    glm::vec3 eye, center, up;
    glm::mat4 viewMatrix = _camera->getViewMatrix(eye, center, up);

    // 获取透视投影矩阵
    float fov, aspect, near, far;
    glm::mat4 projectionMatrix = _camera->getProjectionMatrix(fov, aspect, near, far);

    glm::mat4 viewProjectionMatrix = projectionMatrix * viewMatrix;
    glm::mat4 viewProjectionModelMatrix = viewProjectionMatrix * modelMatrix; // projectionMatrix * viewMatrix * modelMatrix;

    // 记录已处理过的图层顶点数量
    size_t processNodeVertexSize = 0;

    // 绘制所有节点
    for (const auto &node : _nodes)
    {
        auto shader = node->getShader();
        if (!shader)
        {
            continue;
        }

        shader->setUniform("viewMatrix", std::any(viewMatrix));
        shader->setUniform("projectionMatrix", std::any(projectionMatrix));
        shader->setUniform("viewProjectionMatrix", std::any(viewProjectionMatrix));
        shader->setUniform("viewProjectionModelMatrix", std::any(viewProjectionModelMatrix));

        auto &vertexArray = node->getVertexArray();
        auto &vertexLayouts = node->getVertexLayouts();
        auto stride = node->getStride();

        auto ptr = vertexArray.data();

        // 处理顶点属性
        // 每一组stride组成一组顶点属性（pos、color、uv、normal....等）
        for (size_t i = 0; i < vertexArray.size(); i += stride)
        {
            glm::vec4 gl_Position;
            int currentVertexIndex = i / stride;

            for (auto &layoutData : vertexLayouts)
            {
                auto baseOffset = i + layoutData.offset;
                auto dataSize = layoutData.vertexSize;
                shader->setVertexAttrArrayInput(layoutData.layoutId, std::span<const float>(ptr + baseOffset, dataSize));
            }

            // 调用顶点着色器
            auto outputIndex = processNodeVertexSize + currentVertexIndex;
            shader->setVertexAttrArrayOutput(std::span<float>(vertexShaderOutArray[outputIndex].data(), MAX_VERTEX_OUTPUT_MEMORY_SIZE));

            shader->vertexShader(gl_Position);
            gl_PositionArray[outputIndex] = gl_Position;
        }

        // 图元处理
        auto &vertexIndexArray = node->getVertexIndexArray();
        for (size_t vertexIndex = 0; vertexIndex < vertexIndexArray.size(); vertexIndex += 3)
        {
            Triangle tri(
                gl_PositionArray[processNodeVertexSize + vertexIndexArray[vertexIndex]],
                gl_PositionArray[processNodeVertexSize + vertexIndexArray[vertexIndex + 1]],
                gl_PositionArray[processNodeVertexSize + vertexIndexArray[vertexIndex + 2]]);

            auto &vsOutAttr1 = vertexShaderOutArray[processNodeVertexSize + vertexIndexArray[vertexIndex]];
            auto &vsOutAttr2 = vertexShaderOutArray[processNodeVertexSize + vertexIndexArray[vertexIndex + 1]];
            auto &vsOutAttr3 = vertexShaderOutArray[processNodeVertexSize + vertexIndexArray[vertexIndex + 2]];

            // 裁剪视锥体(先省略)
            // 计算平面与顶点关系：
            /*
                对于每个顶点，检查它是否在平面的“可见区域”内（例如：x + w >= 0）。
                如果在内侧，保留该顶点；如果在外侧，计算与该平面交点。
            计算交点：
                对于每一条边，判断两端点是否在平面的两侧（即：一个在内侧，另一个在外侧）。
                如果是，计算交点并将交点与原本的顶点一起保留下来。
            裁剪后的三角形：
                剩下的点和交点会重新组合成裁剪后的三角形。如果一个三角形被分割成两个，则需要拆分。*/

            // 透视除法  在 NDC 中，坐标范围被归一化到([-1, 1])的标准区间
            tri._ndc_position[0] = tri._position[0] / tri._position[0].w;
            tri._ndc_position[1] = tri._position[1] / tri._position[1].w;
            tri._ndc_position[2] = tri._position[2] / tri._position[2].w;

            // 正面/背面剔除
            if (1)
            {
                auto front = calculateFrontFace2D(tri._ndc_position[0], tri._ndc_position[1], tri._ndc_position[2]);

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

            // 视口变换  将 NDC 坐标映射到屏幕坐标 屏幕坐标的范围是[0, width-1]和[0, height-1]
            // 注意：y 坐标需要取反，因为 OpenGL 中的 y 轴是从下到上的
            tri._screen_position[0].x = (tri._ndc_position[0].x + 1) * (_width - 1) / 2;
            tri._screen_position[0].y = (-tri._ndc_position[0].y + 1) * (_height - 1) / 2;

            tri._screen_position[1].x = (tri._ndc_position[1].x + 1) * (_width - 1) / 2;
            tri._screen_position[1].y = (-tri._ndc_position[1].y + 1) * (_height - 1) / 2;

            tri._screen_position[2].x = (tri._ndc_position[2].x + 1) * (_width - 1) / 2;
            tri._screen_position[2].y = (-tri._ndc_position[2].y + 1) * (_height - 1) / 2;

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
                    std::fill_n(_sampleCoveredState.begin(), MSAA_SAMPLE_COUNT, false);

                    for (int sampleIndex = 0; sampleIndex < MSAA_SAMPLE_COUNT; sampleIndex++)
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
                        if (1 && depth > _msaaDepthBuffer[pixelIndex][sampleIndex])
                        {
                            continue;
                        }

                        isCovered = true;
                        _sampleCoveredState[sampleIndex] = true;

                        // 写入深度缓冲区
                        if (1)
                        {
                            _msaaDepthBuffer[pixelIndex][sampleIndex] = depth;
                        }
                    }

                    // 确认写入颜色缓冲区
                    if (isCovered)
                    {
                        glm::vec4 color = glm::vec4(0.0f);

                        // 光栅化输出布局
                        glm::vec4 rasterizeData = glm::vec4(0.0f); // 临时计算数据
                        std::vector<float> rasterizeLayoutOut(MAX_VERTEX_OUTPUT_MEMORY_SIZE, 0.0f);

                        // msaa只插值中心像素点的颜色，然后将颜色复制给被三角形覆盖后的采样点
                        glm::vec3 weights = barycentric(tri._screen_position[0], tri._screen_position[1], tri._screen_position[2], x + 0.5, y + 0.5);
                        float interpolated_w = 1.0 / (weights.x * w0_inv + weights.y * w1_inv + weights.z * w2_inv);

                        for (int loc = 0; loc < MAX_VERTEX_OUTPUT_COMPONENTS; loc++)
                        {
                            auto &meta = shader->getOutputLayout()[loc];
                            if (meta.floatCount == 0)
                            {
                                continue;
                            }

                            int offset = loc * MAX_VERTEX_ATTR_DATA_SIZE;

                            const glm::vec4 &v1 = *reinterpret_cast<const glm::vec4 *>(vsOutAttr1.data() + offset);
                            const glm::vec4 &v2 = *reinterpret_cast<const glm::vec4 *>(vsOutAttr2.data() + offset);
                            const glm::vec4 &v3 = *reinterpret_cast<const glm::vec4 *>(vsOutAttr3.data() + offset);

                            if (meta.interpolation == Shader::Interpolation::Smooth)
                            {
                                rasterizeData =
                                    (weights.x * (v1 * w0_inv) +
                                     weights.y * (v2 * w1_inv) +
                                     weights.z * (v3 * w2_inv)) *
                                    interpolated_w;
                            }
                            else
                            {
                                rasterizeData = v3;
                            }

                            std::memcpy(rasterizeLayoutOut.data() + offset, &rasterizeData, sizeof(glm::vec4));
                        }

                        // 调用片元着色器
                        glm::vec4 gl_FragColor = glm::vec4(0.0f);
                        shader->setFragmentAttrArrayInput(std::span<const float>(rasterizeLayoutOut.data(), MAX_VERTEX_OUTPUT_MEMORY_SIZE));
                        shader->fragmentShader(gl_FragColor);
                        color = gl_FragColor;

                        // 写入颜色缓冲区，只更新深度测试通过的采样点
                        for (int sampleIndex = 0; sampleIndex < MSAA_SAMPLE_COUNT; sampleIndex++)
                        {
                            if (_sampleCoveredState[sampleIndex])
                            {
                                // 开启颜色混合
                                if (1)
                                {
                                    auto oldColor = _msaaColorBuffer[pixelIndex][sampleIndex];
                                    color = alphaBlend(color, oldColor);
                                }

                                // 开启颜色写入
                                if (1)
                                {
                                    _msaaColorBuffer[pixelIndex][sampleIndex] = color;
                                }
                            }
                        }
                    }
                }
            }
        }

        processNodeVertexSize += vertexArray.size() / stride;
    }

    // msaa采样解析 写入帧缓冲区
    int pixelIndex = 0;
    // std::cout << _msaaColorBuffer.size() << " " << frameBuffer.width << " " << frameBuffer.height << std::endl;
    for (auto it = _msaaColorBuffer.begin(); it != _msaaColorBuffer.end(); it++)
    {
        std::array<glm::vec4, MSAA_SAMPLE_COUNT> &colorArray = *it;

        glm::vec4 color = glm::vec4(0.0f);
        for (int i = 0; i < MSAA_SAMPLE_COUNT; i++)
        {
            color += colorArray[i];
        }
        color /= MSAA_SAMPLE_COUNT;

        frameBuffer.pixels[pixelIndex * 4] = color.r * 255;
        frameBuffer.pixels[pixelIndex * 4 + 1] = color.g * 255;
        frameBuffer.pixels[pixelIndex * 4 + 2] = color.b * 255;
        frameBuffer.pixels[pixelIndex * 4 + 3] = color.a * 255;

        pixelIndex++;
    }

    #if 0
    QImage img(
        (uchar*)frameBuffer.pixels.data(),
        frameBuffer.width,
        frameBuffer.height,
        QImage::Format_ARGB32
    );

    img.save("test.png");
    #endif

    auto frameEndTime = std::chrono::steady_clock::now();
    _lastFrameTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                         frameEndTime - frameStartTime).count();

    //_renderTime += _lastFrameTime;
    _renderTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                         frameEndTime - firstFrameTime).count();
    _renderCount++;
}
