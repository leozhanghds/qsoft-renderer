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
        auto backIndex = _backIndex.load(std::memory_order_acquire);
        return _buffers[backIndex];
    }

    // UI 线程读
    const FrameBuffer& front()
    {
        auto frontIndex = _frontIndex.load(std::memory_order_acquire);
        return _buffers[frontIndex];
    }

    // Render 完成一帧后调用
    void swap()
    {
        auto frontIndex = _frontIndex.load(std::memory_order_acquire);
        auto backIndex = _backIndex.load(std::memory_order_acquire);
        
        _frontIndex.store(backIndex, std::memory_order_release);
        _backIndex.store(frontIndex, std::memory_order_release);

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

    std::atomic<int> _frontIndex{0};
    std::atomic<int> _backIndex{1};

    std::atomic<bool> _hasNewFrame{false};
};


#endif