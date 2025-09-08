#include "Layer.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

// 交错数组和顶点索引
void Layer::setVertexArray(std::vector<float> &vertexArray, std::vector<unsigned int>& vertexIndexArray)
{
    _vertexArray.resize(vertexArray.size());
    //std::copy(vertexArray.begin(), vertexArray.end(), _vertexArray.begin());
    std::memcpy(_vertexArray.data(), vertexArray.data(), vertexArray.size() * sizeof(float));

    _vertexIndexArray.resize(vertexIndexArray.size());
    //std::copy(vertexIndexArray.begin(), vertexIndexArray.end(), _vertexIndexArray.begin());
    std::memcpy(_vertexIndexArray.data(), vertexIndexArray.data(), vertexIndexArray.size() * sizeof(unsigned int));
}

// 顶点布局（布局编号，属性长度，步长，偏移量。默认属性都是float）
// opengl是基于字节的，所以要乘以sizeof(float)，此处偏移量是基于数组的，不需要乘以sizeof(float)
void Layer::addVertexLayout(int layoutId, int vertexSize,/* int stride,*/ int offset /*int dataType == float*/)
{
    Data data;
    data.layoutId = layoutId;
    data.vertexSize = vertexSize;
    //data.stride = stride;
    data.offset = offset;

    _stride += vertexSize;
    _vertexLayouts.emplace_back(data);
}

void Layer::setShader(std::shared_ptr<Shader> shader)
{
    _shader = shader;
}

