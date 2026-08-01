#include "LightNode.h"

#include <vector>

LightNode::LightNode()
{
    setType(Type::Light);
    setupCube();
}

glm::vec3 LightNode::getPosition() const
{
    return glm::vec3(getModelMatrix()[3]);
}

void LightNode::setPosition(const glm::vec3 &pos)
{
    glm::mat4 m = getModelMatrix();
    m[3] = glm::vec4(pos, 1.0f);
    setModelMatrix(m);
}

// 构造单位立方体：pos(3) + normal(3)，stride=6，法线在末 3 个 float（选中膨胀复用）
void LightNode::setupCube()
{
    const float facePos[6][4][3] = {
        // 后面 (z = -1)
        {{-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1}},
        // 前面 (z =  1)
        {{-1, -1, 1}, {1, -1, 1}, {1, 1, 1}, {-1, 1, 1}},
        // 左面 (x = -1)
        {{-1, -1, -1}, {-1, -1, 1}, {-1, 1, 1}, {-1, 1, -1}},
        // 右面 (x =  1)
        {{1, -1, -1}, {1, -1, 1}, {1, 1, 1}, {1, 1, -1}},
        // 底面 (y = -1)
        {{-1, -1, -1}, {1, -1, -1}, {1, -1, 1}, {-1, -1, 1}},
        // 顶面 (y =  1)
        {{-1, 1, -1}, {1, 1, -1}, {1, 1, 1}, {-1, 1, 1}},
    };

    const float faceNormal[6][3] = {
        {0, 0, -1}, {0, 0, 1}, {-1, 0, 0}, {1, 0, 0}, {0, -1, 0}, {0, 1, 0},
    };

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    unsigned int base = 0;
    for (int f = 0; f < 6; f++)
    {
        for (int v = 0; v < 4; v++)
        {
            vertices.push_back(facePos[f][v][0]);
            vertices.push_back(facePos[f][v][1]);
            vertices.push_back(facePos[f][v][2]);
            vertices.push_back(faceNormal[f][0]);
            vertices.push_back(faceNormal[f][1]);
            vertices.push_back(faceNormal[f][2]);
        }

        indices.push_back(base + 0);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base + 0);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
        base += 4;
    }

    setVertexArray(vertices, indices);
    addVertexLayout(0, 3, 0); // 位置
    addVertexLayout(1, 3, 3); // 法线
    setShader(std::make_shared<LightCubeShader>(getLayoutCount()));
}
