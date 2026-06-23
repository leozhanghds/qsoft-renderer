#ifndef SHADER_H
#define SHADER_H

#pragma once

#include <glm/glm.hpp>

#include <any>
#include <array>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>
#include <span>

#include "ColorBlend.h"

#include "render_export.h"

constexpr int MAX_VERTEX_ATTR_DATA_SIZE = sizeof(glm::vec4) / sizeof(float);
constexpr int MAX_VERTEX_OUTPUT_COMPONENTS = 16;
constexpr int MAX_VERTEX_OUTPUT_MEMORY_SIZE = MAX_VERTEX_ATTR_DATA_SIZE * MAX_VERTEX_OUTPUT_COMPONENTS;

class RENDER_EXPORT Shader : public std::enable_shared_from_this<Shader>
{
public:
    enum Interpolation : int
    {
        Smooth = 0,
        Flat = 1,
    };

    struct AttrMeta
    {
        uint8_t floatCount{0};
        Interpolation interpolation{Smooth};
    };

    Shader(int layoutCount) : _vertexAttrArrayInput(layoutCount) {}
    virtual ~Shader() = default;

    std::shared_ptr<Shader> getSharedPtr()
    {
        return shared_from_this();
    }

    virtual void vertexShader(glm::vec4 &gl_Position) = 0;

    virtual void fragmentShader(glm::vec4 &gl_FragColor) = 0;

    template <typename T>
    void setUniform(const std::string &name, const T &value)
    {
        _uniforms[name] = std::any(value);
    }

    template <typename T>
    T getUniform(const std::string &name)
    {
        auto it = _uniforms.find(name);
        if (it == _uniforms.end())
        {
            return T();
        }
        return std::any_cast<T>(it->second);
    }

    void addTexture(int textureId, std::shared_ptr<Texture> texture)
    {
        _textures[textureId] = texture;
    }

    const std::shared_ptr<Texture> &getTexture(int textureId) { return _textures[textureId]; }

    void setVertexAttrArrayInput(int location, std::span<const float> vertexAttrArrayInput)
    {
        _vertexAttrArrayInput[location] = vertexAttrArrayInput;
    }

    void setVertexAttrArrayOutput(std::span<float> vertexAttrArrayOutput)
    {
        _vertexAttrArrayOutput = vertexAttrArrayOutput;
        std::fill_n(_vertexAttrArrayOutput.data(), _vertexAttrArrayOutput.size(), 0);
    }

    void setFragmentAttrArrayInput(std::span<const float> fragmentAttrArrayInput)
    {
        _fragmentAttrArrayInput = fragmentAttrArrayInput;
    }

    // 顶点着色器
    template <typename In>
    const In &vlayoutIn(int location)
    {
        return *reinterpret_cast<const In *>(_vertexAttrArrayInput[location].data());
    }

    template <typename Out>
    void vlayoutOut(int location, Interpolation interpolation, const Out &data)
    {
        _outputLayout[location].floatCount = sizeof(Out) / sizeof(float);
        _outputLayout[location].interpolation = interpolation;

        size_t offset = location * MAX_VERTEX_ATTR_DATA_SIZE;
        std::memcpy(_vertexAttrArrayOutput.data() + offset, &data, sizeof(Out));
    }

    // 片元着色器
    template <typename In>
    const In &flayoutIn(int location)
    {
        int offset = location * MAX_VERTEX_ATTR_DATA_SIZE;
        return *reinterpret_cast<const In *>(_fragmentAttrArrayInput.data() + offset);
    }

    const std::array<AttrMeta, MAX_VERTEX_OUTPUT_COMPONENTS> &getOutputLayout() const { return _outputLayout; }

private:
    std::unordered_map<std::string, std::any> _uniforms{};

    std::array<AttrMeta, MAX_VERTEX_OUTPUT_COMPONENTS> _outputLayout{};

    // 顶点着色器输入
    std::vector<std::span<const float>> _vertexAttrArrayInput{}; // C++20 新增的span类型 一个视图，可以减少拷贝数量
    // 顶点着色器输出
    std::span<float> _vertexAttrArrayOutput{};

    // 片元着色器输入和输出
    std::span<const float> _fragmentAttrArrayInput{};

    // 纹理
    std::map<int, std::shared_ptr<Texture>> _textures{};
};

#endif // SHADER_H
