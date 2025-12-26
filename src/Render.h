#ifndef RENDER_H
#define RENDER_H

#pragma once

#include <memory>
#include <vector>
#include <chrono>
#include <array>
#include <bitset>
#include <queue>
#include <mutex>

#include "Camera.h"
#include "Node.h"
#include "DoubleBuffer.h"

#include "render_export.h"

// MSAA多重采样
constexpr int MSAA_SAMPLE_COUNT = 4; // 4x4
const static std::array<glm::vec2, MSAA_SAMPLE_COUNT> sampleOffsets = {
    glm::vec2{0.375f, 0.125f},
    glm::vec2{0.125f, 0.375f},
    glm::vec2{0.625f, 0.875f},
    glm::vec2{0.875f, 0.625f}};

#define CLEAR_COLOR_BUFFER 0b0001
#define CLEAR_DEPTH_BUFFER 0b0010
#define CLEAR_STENCIL_BUFFER 0b0100

struct RenderCommand
{
    enum class Type {
        AddNode,
        RemoveNode,
        Resize
    };

    Type type;
    std::shared_ptr<Node> node;
    
    int pixelWidth;
    int pixelHeight;
};


class RENDER_EXPORT Render : public std::enable_shared_from_this<Render>
{
public:
    Render(std::unique_ptr<DoubleBuffer>& buffer, int width = 800, int height = 800);
    virtual ~Render();

    std::shared_ptr<Render> getSharedPtr(){return shared_from_this();}

    ////////////////////////////  Command
    //TODO 修改为命令模式
    void clearBuffer(FrameBuffer& frameBuffer, std::bitset<4> clearFlags = 0b0111);

    void submitCommand(RenderCommand cmd);

    void renderOneFrame();

    //const uint8_t *getFrameBuffer(){return _frameBuffer.data();}

    const std::shared_ptr<Camera> &getCamera(){return _camera;}

    const float getFrameRenderTime(){return _lastFrameTime;}

protected:
    void addNode(std::shared_ptr<Node> node);
    void removeNode(std::shared_ptr<Node> node);

    void resize(int width, int height);

    void processCommands();

    void drawScene(FrameBuffer& frameBuffer);
public:
    int _width;
    int _height;

private:
    float _renderTime = 0.0f;
    size_t _renderCount = 0;
    float _lastFrameTime = 0.0f;

    // 使用OpenGL默认深度范围 [0, 1]
    const float depth_min = 0.0f;
    const float depth_max = 1.0f;

    // 颜色缓冲区
    //std::vector<uint8_t> _frameBuffer;
    std::unique_ptr<DoubleBuffer>& _doubleBuffer;

    // MSAA缓冲区，记录每个像素的4个采样点的深度/颜色/模板
    std::vector<std::array<float, MSAA_SAMPLE_COUNT>> _msaaDepthBuffer{};
    std::vector<std::array<glm::vec4, MSAA_SAMPLE_COUNT>> _msaaColorBuffer{};
    std::vector<std::array<uint8_t, MSAA_SAMPLE_COUNT>> _msaaStencilBuffer{};
    std::array<bool, MSAA_SAMPLE_COUNT> _sampleCoveredState{false, false, false, false};

    // 相机
    std::shared_ptr<Camera> _camera{nullptr};

    // 节点
    std::vector<std::shared_ptr<Node>> _nodes{};
    std::queue<RenderCommand> _commands{};

    std::mutex _cmdMutex;
};

#endif // RENDER_H