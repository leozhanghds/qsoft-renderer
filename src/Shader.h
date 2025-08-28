#ifndef SHADER_H
#define SHADER_H

#pragma once

#include <glm/glm.hpp>

#include <any>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>
#include <span>

// 定义最大的顶点属性输入输出字节数(第1个字节表示大小, 第2个字节0表示插值，1表示不插值，后面每个属性最大是vec4)
// constexpr int MAX_VERTEX_ATTR_SIZE = sizeof(uint8_t) + sizeof(uint8_t) + sizeof(glm::vec4);
constexpr int MAX_VERTEX_ATTR_SIZE = sizeof(float) + sizeof(float) + sizeof(glm::vec4);
// 顶点着色器属性输出数量限制
constexpr int MAX_VERTEX_OUTPUT_COMPONENTS = 16;
// 顶点着色器属性输出内存大小
constexpr int MAX_VERTEX_OUTPUT_MEMORY_SIZE = MAX_VERTEX_ATTR_SIZE * MAX_VERTEX_OUTPUT_COMPONENTS;

// constexpr int MAX_FRAGMENT_OUTPUT_MEMORY_SIZE = sizeof(glm::vec4);//片元着色器属性输出内存大小
// constexpr int GL_MAX_FRAGMENT_OUTPUT_COMPONENTS = 4;                    //片元着色器属性输出数量限制

class Shader : public std::enable_shared_from_this<Shader>
{
public:
    enum Interpolation : int
    {
        Smooth = 0,
        Flat = 1,
    };

    Shader(int layoutCount) : _vertexAttrArrayInput(layoutCount) {}
    virtual ~Shader() = default;

    std::shared_ptr<Shader> getSharedPtr()
    {
        return shared_from_this();
    }

    virtual size_t getOutLayoutCount()
    {
        throw std::runtime_error("getShaderOutSize not implemented");
    };

    virtual std::unordered_map<std::string, std::any> vertexShader(glm::vec4 &gl_Position) = 0;

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
        // char* arr = reinterpret_cast<uint8_t *>(_vertexAttrArrayOutput.data());

        size_t offset = location * MAX_VERTEX_ATTR_SIZE;

        // 大小
        _vertexAttrArrayOutput[offset] = sizeof(Out) / sizeof(float);

        // 插值
        _vertexAttrArrayOutput[offset + 1] = interpolation;

        // 数据
        std::memcpy(_vertexAttrArrayOutput.data() + offset + 2, &data, sizeof(Out));
    }

    // 片元着色器
    template <typename In>
    const In &flayoutIn(int location)
    {
        // return *reinterpret_cast<const In *>(_fragmentAttrArrayInput[location].data());
        int offset = location * MAX_VERTEX_ATTR_SIZE;
        auto dataSize = static_cast<int>(_fragmentAttrArrayInput[offset]);
        if (dataSize == 0)
        {
            //continue;
            //throw std::runtime_error("");
        }
        Shader::Interpolation interpolation = (Shader::Interpolation)_fragmentAttrArrayInput[offset + 1];

        const glm::vec4 value = *reinterpret_cast<const glm::vec4 *>(
            reinterpret_cast<const unsigned char *>(_fragmentAttrArrayInput.data()) + offset + 2 * sizeof(float)); // 2 表示跳过前面两个float

        return value;
    }

private:
    std::unordered_map<std::string, std::any> _uniforms{};

    // 顶点着色器输入和输出
    std::vector<std::span<const float>> _vertexAttrArrayInput{}; // C++20 新增的span类型 一个视图，可以减少拷贝数量

    std::span<float> _vertexAttrArrayOutput{};

    // 片元着色器输入和输出
    std::span<const float> _fragmentAttrArrayInput{};
};

#endif // SHADER_H
