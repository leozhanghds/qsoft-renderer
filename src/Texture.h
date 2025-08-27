#ifndef TEXTURE_H
#define TEXTURE_H

#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <glm/glm.hpp>

class Texture : public std::enable_shared_from_this<Texture>
{
public:
    enum class WrapMode
    {
        Repeat,
        MirroredRepeat,
        ClampToEdge
    };

    Texture(const std::string &path);
    ~Texture() = default;

    std::shared_ptr<Texture> getSharedPtr()
    {
        return shared_from_this();
    }

    glm::vec4 sample(float u, float v, WrapMode mode = WrapMode::Repeat);

    glm::vec4 sampleBilinear(float u, float v, WrapMode mode = WrapMode::Repeat);

    void wrap(float &u, float &v, WrapMode mode);

    int getWidth() { return _width; }
    int getHeight() { return _height; }
    glm::vec4 getPixel(int x, int y) { return _pixels[y * _width + x]; }

private:
    int _width, _height;
    std::vector<glm::vec4> _pixels;
};

#endif // TEXTURE_H