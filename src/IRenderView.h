#pragma once

#include "Render.h"

class IRenderView
{
public:
    inline Render* render() const
    {
        return _render;
    }


    inline void setRender(Render *render)
    {
        _render = render;
    }

protected:
    // std::unique_ptr<Render> _render;
    Render *_render{nullptr};
};
