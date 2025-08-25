#ifndef RENDER_H
#define RENDER_H

#pragma once

#include <vector>

class Render
{
public:
    Render(int width, int height);
    ~Render();

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
    std::vector<uint8_t> _frameBuffer;

    std::vector<float> _depthBuffer;
};

#endif //RENDER_H