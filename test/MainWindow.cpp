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

#include <iostream>

// 打印索引数据
void printIndices(const std::vector<unsigned int> &indices);
// 打印顶点数据
void printVertices(const std::vector<float> &vertices);
// 生成彩色正方体的顶点数据和索引数据
void generateColoredCube(std::vector<float> &vertices, std::vector<unsigned int> &indices);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    addCubeButton = new QPushButton("add cube", this);
    connect(addCubeButton, &QPushButton::clicked, this, &MainWindow::addCubeLayer);

    deleteCubeButton = new QPushButton("delete cube", this);
    connect(deleteCubeButton, &QPushButton::clicked, this, &MainWindow::deleteCubeLayer);

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
        _cubeNode->addVertexLayout(0, 3, 0); // 顶点
        _cubeNode->addVertexLayout(1, 4, 3); // 颜色

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

void MainWindow::resizeEvent(QResizeEvent *event)
{
    _render->submitCommand(RenderCommand(RenderCommand::Type::Resize, nullptr,
                                        _renderWidget->size().width(), _renderWidget->size().height()));
}

/////////////////////
// 生成彩色正方体的顶点数据和索引数据
void generateColoredCube(std::vector<float> &vertices, std::vector<unsigned int> &indices)
{
    // 正方体顶点位置 (边长2，中心在原点)
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

    // 每个顶点的颜色 (RGBA格式)
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

    // 正方体的6个面（每个面由2个三角形组成）
    const unsigned int faces[6][4] = {
        // 后面 (z = -1)：从外部（后方）看逆时针
        {0, 3, 2, 1},  // 0→3→2→1

        // 前面 (z =  1)：从外部（前方）看逆时针
        {4, 5, 6, 7},  // 4→5→6→7

        // 左面 (x = -1)：从外部（左侧）看逆时针
        {0, 4, 7, 3},  // 0→4→7→3

        // 右面 (x =  1)：从外部（右侧）看逆时针
        {1, 2, 6, 5},  // 1→2→6→5

        // 底面 (y = -1)：从外部（下方）看逆时针
        {0, 1, 5, 4},  // 0→1→5→4

        // 顶面 (y =  1)：从外部（上方）看逆时针
        {3, 7, 6, 2}   // 3→7→6→2
    };

    // 每个面的法线方向
    const float face_normals[6][3] = {
        {0.0f, 0.0f, -1.0f}, // 后面
        {0.0f, 0.0f, 1.0f},  // 前面
        {-1.0f, 0.0f, 0.0f}, // 左面
        {1.0f, 0.0f, 0.0f},  // 右面
        {0.0f, -1.0f, 0.0f}, // 下面
        {0.0f, 1.0f, 0.0f}   // 上面
    };

    // 每个面的纹理坐标 (UV)
    const float face_uvs[4][2] = {
        {0.0f, 0.0f}, // 左下
        {1.0f, 0.0f}, // 右下
        {1.0f, 1.0f}, // 右上
        {0.0f, 1.0f}  // 左上
    };

    // 生成纹理坐标 (UV) 数据
    // 每个面有4个顶点，每个顶点有2个UV坐标
    // for (int face = 0; face < 6; face++)
    // {
    //     for (int vertex = 0; vertex < 4; vertex++)
    //     {
    //         uvs.push_back(face_uvs[vertex][0]);
    //         uvs.push_back(face_uvs[vertex][1]);
    //     }
    // }

    // // 生成法线数据
    // // 每个面有4个顶点，每个顶点使用相同的法线（面的法线）
    // for (int face = 0; face < 6; face++)
    // {
    //     for (int vertex = 0; vertex < 4; vertex++)
    //     {
    //         normals.push_back(face_normals[face][0]);
    //         normals.push_back(face_normals[face][1]);
    //         normals.push_back(face_normals[face][2]);
    //     }
    // }

    // 清空并准备容器
    vertices.clear();
    indices.clear();

    // 生成顶点数据（位置 + 颜色）
    for (int i = 0; i < 8; i++)
    {
        // 位置
        vertices.push_back(positions[i][0]);
        vertices.push_back(positions[i][1]);
        vertices.push_back(positions[i][2]);

        // 颜色
        vertices.push_back(colors[i][0]);
        vertices.push_back(colors[i][1]);
        vertices.push_back(colors[i][2]);
        vertices.push_back(colors[i][3]);
    }

    // 生成索引数据（每个面由2个三角形组成）
    for (int face = 0; face < 6; face++)
    {
        // 第一个三角形
        indices.push_back(faces[face][0]);
        indices.push_back(faces[face][1]);
        indices.push_back(faces[face][2]);

        // 第二个三角形
        indices.push_back(faces[face][0]);
        indices.push_back(faces[face][2]);
        indices.push_back(faces[face][3]);
    }
}

// 打印顶点数据
void printVertices(const std::vector<float> &vertices)
{
    std::cout << "顶点数据 (位置 + 颜色):\n";
    for (size_t i = 0; i < vertices.size(); i += 7)
    {
        std::cout << "顶点 " << i / 7 << ": ";
        std::cout << "(" << vertices[i] << ", " << vertices[i + 1] << ", " << vertices[i + 2] << ") ";
        std::cout << "颜色: (" << vertices[i + 3] << ", " << vertices[i + 4] << ", "
                  << vertices[i + 5] << ", " << vertices[i + 6] << ")\n";
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
