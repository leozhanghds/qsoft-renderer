#include "MainWindow.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QDockWidget>

#include "Node.h"
#include "Render.h"
#include "Texture.h"
#include "RenderWidget.h"
#include "RenderThread.h"
#include "DoubleBuffer.h"

#include "SquareCubeShader.h"
#include "BunnyShader.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cmath>
#include <cfloat>
#include <glm/gtc/matrix_transform.hpp>

// 打印索引数据
void printIndices(const std::vector<unsigned int> &indices);
// 打印顶点数据
void printVertices(const std::vector<float> &vertices);
// 生成彩色正方体的顶点数据和索引数据
void generateColoredCube(std::vector<float> &vertices, std::vector<unsigned int> &indices);
bool loadOBJ(const std::string &relativePath, std::vector<float> &vertices, std::vector<unsigned int> &indices, glm::vec3 &center);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    addCubeButton = new QPushButton("add cube", this);
    connect(addCubeButton, &QPushButton::clicked, this, &MainWindow::addCubeLayer);

    deleteCubeButton = new QPushButton("delete cube", this);
    connect(deleteCubeButton, &QPushButton::clicked, this, &MainWindow::deleteCubeLayer);

    addBunnyButton = new QPushButton("add bunny", this);
    connect(addBunnyButton, &QPushButton::clicked, this, &MainWindow::addBunnyLayer);

    deleteBunnyButton = new QPushButton("delete bunny", this);
    connect(deleteBunnyButton, &QPushButton::clicked, this, &MainWindow::deleteBunnyLayer);

    _renderBuffer = std::make_unique<DoubleBuffer>();
    _render = std::make_unique<Render>(_renderBuffer);
    _renderThread = std::make_unique<RenderThread>(_render);
    _renderWidget = std::make_unique<RenderWidget>(_renderBuffer);

    _renderThread->start();

    QDockWidget *leftDock = new QDockWidget("Left Dock", this);
    QWidget *dockContent = new QWidget(leftDock);
    leftDock->setMinimumWidth(160);
    leftDock->setWidget(dockContent); // 将容器设置为停靠窗口的内容

    QVBoxLayout *buttonLayout = new QVBoxLayout(dockContent);
    buttonLayout->setContentsMargins(5, 5, 5, 5); // 上下左右边距
    buttonLayout->setSpacing(5);                  // 按钮间距
    buttonLayout->addWidget(addCubeButton);
    buttonLayout->addWidget(deleteCubeButton);
    buttonLayout->addWidget(addBunnyButton);
    buttonLayout->addWidget(deleteBunnyButton);
    buttonLayout->addStretch();

    this->setCentralWidget(_renderWidget.get());
    this->addDockWidget(Qt::LeftDockWidgetArea, leftDock);

    this->resize(640, 560);

    _sharedTexture = std::make_shared<Texture>("textures/container.jpg");

    if (1)
    {
        // 渲染一个正方形
        std::vector<float> vertexArray{
            // pos  //color // uv
            -2.f, 0, 0, 1.f, 0, 0, 1.f, 0, 0, // left button
            2.f, 0, 0, 0.f, 1, 0, 1.f, 1, 0,  // right button
            2.f, 2, 0, 0.f, 0, 1, 1.f, 1, 1,  // right top
            -2.f, 2, 0, 0.f, 0, 1, 1.f, 0, 1  // left top
        };

        std::vector<unsigned int> vertexIndexArray{
            0, 1, 2,
            0, 2, 3};

        std::shared_ptr<Node> node = std::make_shared<Node>();
        node->setVertexArray(vertexArray, vertexIndexArray);
        node->addVertexLayout(0, 3, 0); // 顶点
        node->addVertexLayout(1, 4, 3); // 颜色
        node->addVertexLayout(2, 2, 7); // uv

        auto shader = std::make_shared<SquareShader>(node->getLayoutCount());
        shader->addTexture(0, _sharedTexture);

        node->setShader(shader);
        _render->submitCommand(RenderCommand(RenderCommand::Type::AddNode, node));
    }

    // 设置相机回调
    _render->getCamera()->setUpdateCallback([this](std::shared_ptr<Camera> camera){
        glm::vec3 eye, center, up;
        camera->getViewMatrix(eye, center, up);

        float radius = 6.0f;
        float angularSpeed = 0.001f;  // rad/ms
        float camX = sin(camera->getRenderTime() * angularSpeed) * radius;
        float camZ = cos(camera->getRenderTime() * angularSpeed) * radius;
        eye = glm::vec3(camX, 5.0f, camZ); // 周期运动
        camera->setViewMatrix(eye, center, up); 
    });
}

MainWindow::~MainWindow()
{
    _renderThread->stop();
}

void MainWindow::addCubeLayer()
{
    if (!_cubeNode)
    {
        std::vector<float> vertexArray;
        std::vector<unsigned int> vertexIndexArray;
        generateColoredCube(vertexArray, vertexIndexArray);
        printVertices(vertexArray);
        printIndices(vertexIndexArray);

        _cubeNode = std::make_shared<Node>();
        _cubeNode->setVertexArray(vertexArray, vertexIndexArray);
        _cubeNode->addVertexLayout(0, 3, 0);  // 顶点
        _cubeNode->addVertexLayout(1, 4, 3);  // 颜色
        _cubeNode->addVertexLayout(2, 2, 7);  // uv
        _cubeNode->addVertexLayout(3, 3, 9);  // 法线

        auto shader = std::make_shared<CubeShader>(_cubeNode->getLayoutCount());
        shader->addTexture(0, _sharedTexture);

        _cubeNode->setShader(shader);
        _render->submitCommand(RenderCommand(RenderCommand::Type::AddNode, _cubeNode));
    }
}

void MainWindow::deleteCubeLayer()
{
    if (_cubeNode)
    {
        _render->submitCommand(RenderCommand(RenderCommand::Type::RemoveNode, _cubeNode));
        _cubeNode.reset();
    }
}

void MainWindow::addBunnyLayer()
{
    if (!_bunnyNode)
    {
        std::vector<float> vertexArray;
        std::vector<unsigned int> vertexIndexArray;
        glm::vec3 center;

        if (!loadOBJ("obj/bunny.obj", vertexArray, vertexIndexArray, center))
        {
            std::cerr << "Failed to load bunny.obj" << std::endl;
            return;
        }

        _bunnyNode = std::make_shared<Node>();
        _bunnyNode->setVertexArray(vertexArray, vertexIndexArray);
        _bunnyNode->addVertexLayout(0, 3, 0);
        _bunnyNode->addVertexLayout(1, 3, 3);

        auto shader = std::make_shared<BunnyShader>(_bunnyNode->getLayoutCount());

        float scale = 15.0f;
        glm::mat4 modelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scale));
        modelMatrix = glm::translate(modelMatrix, -center);
        shader->setUniform("modelMatrix", modelMatrix);

        _bunnyNode->setShader(shader);
        _render->submitCommand(RenderCommand(RenderCommand::Type::AddNode, _bunnyNode));
    }
}

void MainWindow::deleteBunnyLayer()
{
    if (_bunnyNode)
    {
        _render->submitCommand(RenderCommand(RenderCommand::Type::RemoveNode, _bunnyNode));
        _bunnyNode.reset();
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    _render->submitCommand(RenderCommand(RenderCommand::Type::Resize, nullptr,
                                        _renderWidget->size().width(), _renderWidget->size().height()));
}

// 生成彩色正方体的顶点数据和索引数据
void generateColoredCube(std::vector<float> &vertices, std::vector<unsigned int> &indices)
{
    const float positions[8][3] = {
        {-1.0f, -1.0f, -1.0f}, // 0: 左后下
        {1.0f, -1.0f, -1.0f},  // 1: 右后下
        {1.0f, 1.0f, -1.0f},   // 2: 右后上
        {-1.0f, 1.0f, -1.0f},  // 3: 左后上
        {-1.0f, -1.0f, 1.0f},  // 4: 左前下
        {1.0f, -1.0f, 1.0f},   // 5: 右前下
        {1.0f, 1.0f, 1.0f},    // 6: 右前上
        {-1.0f, 1.0f, 1.0f}    // 7: 左前上
    };

    const float colors[8][4] = {
        {1.0f, 0.0f, 0.0f, 1.0f}, // 0: 红色
        {0.0f, 1.0f, 0.0f, 1.0f}, // 1: 绿色
        {0.0f, 0.0f, 1.0f, 1.0f}, // 2: 蓝色
        {1.0f, 1.0f, 0.0f, 1.0f}, // 3: 黄色
        {1.0f, 0.0f, 1.0f, 1.0f}, // 4: 紫色
        {0.0f, 1.0f, 1.0f, 1.0f}, // 5: 青色
        {1.0f, 0.5f, 0.0f, 1.0f}, // 6: 橙色
        {0.5f, 0.0f, 0.5f, 1.0f}  // 7: 深紫色
    };

    const unsigned int faces[6][4] = {
        {0, 3, 2, 1}, // 后面 (z = -1)
        {4, 5, 6, 7}, // 前面 (z =  1)
        {0, 4, 7, 3}, // 左面 (x = -1)
        {1, 2, 6, 5}, // 右面 (x =  1)
        {0, 1, 5, 4}, // 底面 (y = -1)
        {3, 7, 6, 2}  // 顶面 (y =  1)
    };

    const float face_normals[6][3] = {
        {0.0f, 0.0f, -1.0f}, // 后面
        {0.0f, 0.0f, 1.0f},  // 前面
        {-1.0f, 0.0f, 0.0f}, // 左面
        {1.0f, 0.0f, 0.0f},  // 右面
        {0.0f, -1.0f, 0.0f}, // 下面
        {0.0f, 1.0f, 0.0f}   // 上面
    };

    const float face_uvs[4][2] = {
        {0.0f, 0.0f}, // 左下
        {1.0f, 0.0f}, // 右下
        {1.0f, 1.0f}, // 右上
        {0.0f, 1.0f}  // 左上
    };

    vertices.clear();
    indices.clear();

    for (int face = 0; face < 6; face++)
    {
        unsigned int base = face * 4;

        for (int v = 0; v < 4; v++)
        {
            int pi = faces[face][v];

            // pos (3)
            vertices.push_back(positions[pi][0]);
            vertices.push_back(positions[pi][1]);
            vertices.push_back(positions[pi][2]);

            // color (4)
            vertices.push_back(colors[pi][0]);
            vertices.push_back(colors[pi][1]);
            vertices.push_back(colors[pi][2]);
            vertices.push_back(colors[pi][3]);

            // uv (2)
            vertices.push_back(face_uvs[v][0]);
            vertices.push_back(face_uvs[v][1]);

            // normal (3)
            vertices.push_back(face_normals[face][0]);
            vertices.push_back(face_normals[face][1]);
            vertices.push_back(face_normals[face][2]);
        }

        indices.push_back(base + 0);
        indices.push_back(base + 1);
        indices.push_back(base + 2);

        indices.push_back(base + 0);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
    }
}

// 打印顶点数据
void printVertices(const std::vector<float> &vertices)
{
    std::cout << "顶点数据 (位置 + 颜色 + UV + 法线):\n";
    for (size_t i = 0; i < vertices.size(); i += 12)
    {
        std::cout << "顶点 " << i / 12 << ": ";
        std::cout << "pos(" << vertices[i] << ", " << vertices[i + 1] << ", " << vertices[i + 2] << ") ";
        std::cout << "color(" << vertices[i + 3] << ", " << vertices[i + 4] << ", "
                  << vertices[i + 5] << ", " << vertices[i + 6] << ") ";
        std::cout << "uv(" << vertices[i + 7] << ", " << vertices[i + 8] << ") ";
        std::cout << "normal(" << vertices[i + 9] << ", " << vertices[i + 10] << ", " << vertices[i + 11] << ")\n";
    }
}

// 打印索引数据
void printIndices(const std::vector<unsigned int> &indices)
{
    std::cout << "\n索引数据 (每个面2个三角形):\n";
    for (size_t i = 0; i < indices.size(); i += 3)
    {
        std::cout << "三角形 " << i / 3 << ": ";
        std::cout << indices[i] << ", " << indices[i + 1] << ", " << indices[i + 2] << "\n";
    }
}

bool loadOBJ(const std::string &relativePath, std::vector<float> &vertices, std::vector<unsigned int> &indices, glm::vec3 &center)
{
    std::string fullPath;
    const char *env = std::getenv("RENDER_RESOURCE_PATH");
    if (env)
    {
        fullPath = std::string(env) + "/" + relativePath;
    }
    else
    {
        fullPath = relativePath;
    }

    std::ifstream file(fullPath);
    if (!file.is_open())
    {
        std::cerr << "Failed to open OBJ: " << fullPath << std::endl;
        return false;
    }

    std::vector<glm::vec3> positions;
    std::vector<glm::ivec3> faces;

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '#')
            continue;

        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v")
        {
            glm::vec3 pos;
            iss >> pos.x >> pos.y >> pos.z;
            positions.push_back(pos);
        }
        else if (prefix == "f")
        {
            int i0, i1, i2;
            iss >> i0 >> i1 >> i2;
            faces.push_back(glm::ivec3(i0 - 1, i1 - 1, i2 - 1));
        }
    }

    glm::vec3 minPos(FLT_MAX);
    glm::vec3 maxPos(-FLT_MAX);
    for (auto &p : positions)
    {
        minPos = glm::min(minPos, p);
        maxPos = glm::max(maxPos, p);
    }
    center = (minPos + maxPos) * 0.5f;

    vertices.clear();
    indices.clear();

    unsigned int idx = 0;
    for (auto &face : faces)
    {
        glm::vec3 &p0 = positions[face.x];
        glm::vec3 &p1 = positions[face.y];
        glm::vec3 &p2 = positions[face.z];

        glm::vec3 normal = glm::normalize(glm::cross(p1 - p0, p2 - p0));
        if (std::isnan(normal.x))
        {
            normal = glm::vec3(0.0f, 1.0f, 0.0f);
        }

        float verts[] = {
            p0.x, p0.y, p0.z, normal.x, normal.y, normal.z,
            p1.x, p1.y, p1.z, normal.x, normal.y, normal.z,
            p2.x, p2.y, p2.z, normal.x, normal.y, normal.z,
        };
        vertices.insert(vertices.end(), std::begin(verts), std::end(verts));

        indices.push_back(idx);
        indices.push_back(idx + 1);
        indices.push_back(idx + 2);
        idx += 3;
    }

    std::cout << "Loaded OBJ: " << positions.size() << " positions, "
              << faces.size() << " faces, "
              << vertices.size() / 6 << " expanded vertices" << std::endl;

    return true;
}
