#include "Node.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/geometric.hpp>

// 交错数组和顶点索引
void Node::setVertexArray(std::vector<float> &vertexArray, std::vector<unsigned int>& vertexIndexArray)
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
void Node::addVertexLayout(int layoutId, int vertexSize,/* int stride,*/ int offset /*int dataType == float*/)
{
    Data data;
    data.layoutId = layoutId;
    data.vertexSize = vertexSize;
    //data.stride = stride;
    data.offset = offset;

    _stride += vertexSize;
    _vertexLayouts.emplace_back(data);
}

void Node::setShader(std::shared_ptr<Shader> shader)
{
    _shader = shader;
}

void Node::applyVertexExpansion(const std::vector<float> &originalVertexArray, float amount)
{
    if (originalVertexArray.size() != _vertexArray.size() || _stride < 6 || amount == 0.0f)
        return;

    // 法线固定为 stride 的最后 3 个 float
    const int normalOffset = _stride - 3;

    for (size_t i = 0; i < _vertexArray.size(); i += _stride)
    {
        glm::vec3 pos(originalVertexArray[i], originalVertexArray[i + 1], originalVertexArray[i + 2]);
        glm::vec3 nrm(originalVertexArray[i + normalOffset],
                      originalVertexArray[i + normalOffset + 1],
                      originalVertexArray[i + normalOffset + 2]);

        float len = glm::length(nrm);
        if (len < 1e-6f)
            continue;
        nrm /= len;
        pos += nrm * amount;

        _vertexArray[i]     = pos.x;
        _vertexArray[i + 1] = pos.y;
        _vertexArray[i + 2] = pos.z;
    }
}

void Node::restoreVertexArray(const std::vector<float> &originalVertexArray)
{
    if (originalVertexArray.size() == _vertexArray.size())
        _vertexArray = originalVertexArray;
}

