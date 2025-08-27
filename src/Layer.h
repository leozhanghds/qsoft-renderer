#ifndef LAYER_H
#define LAYER_H

#pragma once

#include <memory>
#include <vector>

#include "Texture.h"

class Layer
{
public:
    struct Data
    {
        int layoutId;
        int vertexSize;
        //int stride;
        int offset;
    };

    Layer() {}
    ~Layer() {}

    // 交错数组和顶点索引
    void setVertexArray(std::vector<float> &vertexArray, std::vector<unsigned int> &vertexIndexArray);

    // 顶点布局（布局编号，属性长度，步长，偏移量。默认属性都是float）
    // 由于交错数组的stride都是一样的，所以整个图层设置了一个stride就足够了
    void addVertexLayout(int layoutId, int vertexSize, /*int stride,*/ int offset /*int dataType == float*/);

    // 步长
    void setStride(int stride) { _stride = stride; }
    
    int getStride() { return _stride; }

    // 纹理
    void setTexture(std::shared_ptr<Texture> texture);

    /////////////////////////////////////////////////////////////////////

    // 顶点布局
    const std::vector<Data> &getVertexLayouts() { return _vertexLayouts; }

    // 顶点索引数组
    const std::vector<unsigned int> &getVertexIndexArray() { return _vertexIndexArray; }

    // 顶点数组
    const std::vector<float> &getVertexArray() { return _vertexArray; }

    const std::shared_ptr<Texture> &getTexture() { return _texture; }

private:
    std::vector<float> _vertexArray;
    std::vector<unsigned int> _vertexIndexArray;

    std::vector<Data> _vertexLayouts;

    std::shared_ptr<Texture> _texture;

    // 步长
    int _stride;
};

#endif // LAYER_H
