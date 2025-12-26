#pragma once

#include <memory>

#include "DoubleBuffer.h"

class IRenderView
{
public:
    IRenderView(std::unique_ptr<DoubleBuffer>& doubleBuffer) : _doubleBuffer(doubleBuffer) {}

    virtual ~IRenderView() = default;

protected:
    std::unique_ptr<DoubleBuffer>& _doubleBuffer;
};
