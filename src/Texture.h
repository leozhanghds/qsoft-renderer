#ifndef TEXTURE_H
#define TEXTURE_H

#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <glm/glm.hpp>

#include "render_export.h"

class RENDER_EXPORT Texture : public std::enable_shared_from_this<Texture>
{
public:
    enum class WrapMode
    {
        Repeat,
        MirroredRepeat,
        ClampToEdge
    };

    Texture(const std::string &path);
    virtual ~Texture() = default;

    std::shared_ptr<Texture> getSharedPtr()
    {
        return shared_from_this();
    }

    glm::vec4 sample(glm::vec2 uv);

    glm::vec4 sample(float u, float v);

    glm::vec4 sampleBilinear(glm::vec2 uv);

    glm::vec4 sampleBilinear(float u, float v);

    void wrap(float &d, WrapMode mode);

    int getWidth() { return _width; }
    int getHeight() { return _height; }
    glm::vec4 getPixel(int x, int y) { return _pixels[y * _width + x]; }

    void setWrapMode(WrapMode umode = WrapMode::Repeat, WrapMode vMode = WrapMode::Repeat)
    {
        _wrapModeU = umode;
        _wrapModeV = vMode;
    }

private:
    int _width, _height;
    std::vector<glm::vec4> _pixels;

    WrapMode _wrapModeU{Texture::WrapMode::Repeat};

    WrapMode _wrapModeV{Texture::WrapMode::Repeat};
};

#endif // TEXTURE_H