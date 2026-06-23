#include "RenderThread.h"

#include "Render.h"


RenderThread::RenderThread(std::unique_ptr<Render> &render) 
    : _render(render)
{
}

RenderThread::~RenderThread()
{
}

void RenderThread::start()
{
    _running = true;
    _thread = std::thread(&RenderThread::loop, this);
}

void RenderThread::stop()
{
    _running = false;
    if (_thread.joinable())
    {
        _thread.join();
    }
}

void RenderThread::loop()
{
    auto next = std::chrono::steady_clock::now();
    while (_running)
    {
        _render->renderOneFrame();
        //next += std::chrono::milliseconds(32);
        next += std::chrono::milliseconds(16);
        std::this_thread::sleep_until(next);
    }
}
