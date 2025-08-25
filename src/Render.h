#ifndef RENDER_H
#define RENDER_H

#pragma once

#include <vector>
#include <chrono>

class Render
{
public:
    Render(int width, int height);
    ~Render();

    void initVertexArray(std::vector<float>&& vertexArray,std::vector<unsigned int> vertexIndexArray);

    void clear();

    void testRender();

    void frame();

    const uint8_t* getFrameBuffer() {
        return _frameBuffer.data();
    }

public:
    int _width;
    int _height;

private:
    std::chrono::time_point<std::chrono::steady_clock> _startTime;

    std::vector<uint8_t> _frameBuffer;

    std::vector<float> _depthBuffer;

    std::vector<float> _vertexArray;

    std::vector<unsigned int> _vertexIndexArray;
};

#endif //RENDER_H