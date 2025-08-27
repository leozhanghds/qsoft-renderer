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

class Shader : public std::enable_shared_from_this<Shader>
{
public:
    Shader(int layoutCount) : _vertexAttrArrayInput(layoutCount, std::vector<float>(4)) {}
    virtual ~Shader() = default;

    std::shared_ptr<Shader> getSharedPtr()
    {
        return shared_from_this();
    }

    virtual std::unordered_map<std::string, std::any> vertexShader(glm::vec4 &gl_Position) = 0;

    virtual void fragmentShader() = 0;

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

    std::vector<std::vector<float>> &getVertexAttrArrayInput()
    {
        return _vertexAttrArrayInput;
    }

    template <typename U>
    U layout(int layoutId)
    {
        return *reinterpret_cast<U *>(_vertexAttrArrayInput[layoutId].data());
    }

private:
    std::unordered_map<std::string, std::any> _uniforms{};

    std::vector<std::vector<float>> _vertexAttrArrayInput{};
};

#endif // SHADER_H
