#ifndef RENDER_H
#define RENDER_H

#pragma once

#include <memory>
#include <vector>
#include <chrono>

//class Texture;
#include "Texture.h"

class Render
{
public:
    Render(int width, int height);
    ~Render();

    void initVertexArray(std::vector<float> &&vertexArray, std::vector<unsigned int> vertexIndexArray);

    void setTexture(std::shared_ptr<Texture> texture);

    void setLayout(int vertexLayout, int vertexSize, int colorLayout, int colorSize, int uvLayout, int uvSize);

    //void setWrapMode(Texture::WrapMode wrapMode);

    void clear();

    void frame();

    const uint8_t *getFrameBuffer()
    {
        return _frameBuffer.data();
    }

public:
    int _width;
    int _height;

private:
    std::chrono::time_point<std::chrono::steady_clock> _startTime;

    // 使用OpenGL默认深度范围 [0, 1]
    const float depth_min = 0.0f;
    const float depth_max = 1.0f;

    // 颜色缓冲区
    std::vector<uint8_t> _frameBuffer;

    // 深度缓冲区
    std::vector<float> _depthBuffer;

    // 顶点数组
    std::vector<float> _vertexArray;

    // 顶点索引数组
    std::vector<unsigned int> _vertexIndexArray;

    std::shared_ptr<Texture> _texture;

    // 每个顶点大小
    int _vertexLayout = 0;
    int _vertexSize = 3;

    // 每个颜色大小
    int _colorLayout = 0;
    int _colorSize = 4;

    // 每个uv大小
    int _uvSize = 2;
    int _uvLayout = 0;
};

#endif // RENDER_H