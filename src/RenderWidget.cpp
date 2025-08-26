#include "RenderWidget.h"
#include "Render.h"

#include <iostream>

#include <QThread>
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <QApplication>

#define WIDGET_WIDTH 800
#define WIDGET_HEIGHT 800

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
        {0, 1, 2, 3}, // 后面
        {4, 5, 6, 7}, // 前面
        {3, 7, 4, 0}, // 左面
        {1, 5, 6, 2}, // 右面
        {0, 4, 5, 1}, // 下面
        {3, 2, 6, 7}  // 上面
    };

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
///////////////////

RenderWidget::RenderWidget(QWidget *parent)
    : QWidget(parent)
{
    _render = std::make_unique<Render>(WIDGET_WIDTH, WIDGET_HEIGHT);
    _displayImage = QImage((uchar *)_render->getFrameBuffer(), _render->_width, _render->_height, QImage::Format_ARGB32);

    setWindowTitle("RenderWidget");
    setFixedSize(_render->_width, _render->_height);

    #if 0
    std::vector<float> vertexArray{
        // pos         //color
        -1.f, 0, 0, 1.f, 0, 0, 1.f, // left button
        1.f, 0, 0, 0.f, 1, 0, 1.f,  // right button
        1.f, 1, 0, 0.f, 0, 1, 1.f,  // right top
        -1.f, 1, 0, 0.f, 0, 1, 1.f  // left top
    };

    std::vector<int> vertexIndexArray{
        0, 1, 2,
        0, 2, 3};

    #else
    std::vector<float> vertexArray;
    std::vector<unsigned int> vertexIndexArray;
    generateColoredCube(vertexArray, vertexIndexArray);
    printVertices(vertexArray);
    printIndices(vertexIndexArray);
    #endif

    _render->initVertexArray(std::move(vertexArray), std::move(vertexIndexArray));
}

RenderWidget::~RenderWidget()
{
}

void RenderWidget::render()
{
    int frameCount = 0;
    while (true)
    {
        QApplication::processEvents(); // 处理ui事件
        _render->frame();
        update();
        
        //_displayImage.save(QString("test-%1.png").arg(frameCount));
        // frameCount++;
        // if (frameCount > 10)
        // {
        //     break;
        // }

        QThread::msleep(16);
    }
}

void RenderWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.drawImage(0, 0, _displayImage);
}

void RenderWidget::mousePressEvent(QMouseEvent *event)
{
    _lastMousePos = event->pos();
}

void RenderWidget::mouseMoveEvent(QMouseEvent *event)
{

    qDebug() << _lastMousePos << event->pos();

    QPoint delta = event->pos() - _lastMousePos;
    _lastMousePos = event->pos();

    // 根据偏移量更新相机角度
    //_renderer.camera.rotate(delta.x(), delta.y());

    update();
}