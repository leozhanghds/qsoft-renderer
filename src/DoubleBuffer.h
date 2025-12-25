#ifndef DOUBLEBUFFER_H
#define DOUBLEBUFFER_H

#pragma once

#include <vector>
#include <mutex>
#include <atomic>

#include <iostream>

struct FrameBuffer
{
    int width = 0;
    int height = 0;
    std::vector<uint8_t> pixels; // ARGB32

    FrameBuffer(int w, int h)
        : width(w), height(h), pixels(w * h * 4)
    {}

    void resize(int w, int h)
    {
        width = w;
        height = h;
        pixels.resize(w * h * 4);
    }
};

class DoubleBuffer
{
public:
    DoubleBuffer(int w = 800, int h = 800)
        : _buffers{
            FrameBuffer(w, h),
            FrameBuffer(w, h)
        }
    {}

    // Render 线程写
    FrameBuffer& back()
    {
        return _buffers[_backIndex];
    }

    // UI 线程读
    const FrameBuffer& front()
    {
        return _buffers[_frontIndex];
    }

    // Render 完成一帧后调用
    void swap()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        std::swap(_frontIndex, _backIndex);
        _hasNewFrame.store(true, std::memory_order_release);
    }

    bool hasNewFrame() const
    {
        return _hasNewFrame.load(std::memory_order_acquire);
    }

    void consumeFrame()
    {
        _hasNewFrame.store(false, std::memory_order_release);
    }

private:
    FrameBuffer _buffers[2];
    int _frontIndex = 0;
    int _backIndex = 1;

    std::atomic<bool> _hasNewFrame{false};
    std::mutex _mutex; // 只保护 swap
};


#endif