#pragma once

#include <memory>
#include <thread>
#include <atomic>
#include <mutex>

#include "render_export.h"

class Render;
class RENDER_EXPORT RenderThread
{
public:
    RenderThread(std::unique_ptr<Render>& render);
    virtual ~RenderThread();

    void start();
    void stop();

protected:
    void loop();

private:   
    std::thread _thread;
    std::atomic<bool> _running{false};
    std::mutex _mutex;

    std::unique_ptr<Render>& _render;
};