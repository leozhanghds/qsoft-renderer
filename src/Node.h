#ifndef NODE_H
#define NODE_H

#pragma once

#include <memory>
#include <vector>
#include <map>

#include "Texture.h"
#include "Shader.h"
#include "ColorBlend.h"

class Node
{
public:
    struct Data
    {
        int layoutId;
        int vertexSize;
        // int stride;
        int offset;
    };

    enum class CullFace
    {
        Back,
        Front,
        FrontAndBack,
    };

    Node() {}
    ~Node() {}

    // 交错数组和顶点索引
    void setVertexArray(std::vector<float> &vertexArray, std::vector<unsigned int> &vertexIndexArray);

    // 顶点布局（布局编号，属性长度，步长，偏移量。默认属性都是float）
    // 由于交错数组的stride都是一样的，所以整个图层设置了一个stride就足够了
    void addVertexLayout(int layoutId, int vertexSize, /*int stride,*/ int offset /*int dataType == float*/);

    // 步长
    const int getStride() { return _stride; }

    const int getLayoutCount() { return _vertexLayouts.size(); }

    // 着色器
    void setShader(std::shared_ptr<Shader> shader);

    /////////////////////////////////////////////////////////////////////

    const std::vector<Data> &getVertexLayouts() { return _vertexLayouts; }

    const std::vector<unsigned int> &getVertexIndexArray() { return _vertexIndexArray; }

    const std::vector<float> &getVertexArray() { return _vertexArray; }

    const std::shared_ptr<Shader> &getShader() { return _shader; }

public:
    // 深度测试（枚举暂时省略）
    bool depthTest{true};

    // 深度写入
    bool depthWrite{false};

    // 深度函数
    //DepthFunc depthFunc{DepthFunc::Less};

    // 颜色混合
    bool blend{false};

    // 颜色混合函数
    BlendFactor srcFactor{BlendFactor::SRC_ALPHA};
    BlendFactor dstFactor{BlendFactor::ONE_MINUS_SRC_ALPHA};

    // 颜色写入
    bool colorWrite{true};

    // 面剔除
    bool cullFace{true};

    // 面剔除方向
    CullFace cullFaceDir{CullFace::Back};

private:
    std::vector<float> _vertexArray;
    std::vector<unsigned int> _vertexIndexArray;

    std::vector<Data> _vertexLayouts;

    std::shared_ptr<Shader> _shader{nullptr};

    // 步长
    int _stride{0};
};

#endif // NODE_H
