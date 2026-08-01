#include "RenderWidget.h"
#include "Render.h"
#include "Node.h"
#include "Texture.h"

#include <QThread>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDebug>
#include <QApplication>

RenderWidget::RenderWidget(std::unique_ptr<DoubleBuffer>& buffer) : QWidget(nullptr), IRenderView(buffer)
{
    _renderTimer = new QTimer(this);
    connect(_renderTimer, &QTimer::timeout, [this](){
        if (_doubleBuffer->hasNewFrame()){    
            update();
            _doubleBuffer->consumeFrame();
        }
    });
    _renderTimer->start(16);
}

RenderWidget::~RenderWidget()
{}

void RenderWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QWidget::paintEvent(event);

    const auto& buffer = _doubleBuffer->front();
    QImage img(
        (uchar*)buffer.pixels.data(),
        buffer.width,
        buffer.height,
        QImage::Format_RGBA8888 //注意通道顺序
    );

    QPainter painter(this);    
    painter.drawImage(rect(), img);
}

void RenderWidget::mousePressEvent(QMouseEvent *event)
{
    if (!_render)
        return;

    if (event->button() == Qt::RightButton)
    {
        _lastMousePos = event->pos();
        RenderCommand cmd;
        cmd.type = RenderCommand::Type::SelectNode;
        cmd.pixelX = event->pos().x();
        cmd.pixelY = event->pos().y();
        _render->submitCommand(std::move(cmd));
    }
    else if (event->button() == Qt::LeftButton)
    {
        _lastMousePos = event->pos();
    }
}

void RenderWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!_render)
        return;

    if (event->buttons() & Qt::RightButton)
    {
        RenderCommand cmd;
        cmd.type = RenderCommand::Type::DragNode;
        cmd.pixelX = event->pos().x();
        cmd.pixelY = event->pos().y();
        _render->submitCommand(std::move(cmd));
    }
    else if (event->buttons() & Qt::LeftButton)
    {
        RenderCommand cmd;
        cmd.type = RenderCommand::Type::PanCamera;
        cmd.pixelX = event->pos().x() - _lastMousePos.x();
        cmd.pixelY = event->pos().y() - _lastMousePos.y();
        _render->submitCommand(std::move(cmd));
        _lastMousePos = event->pos();
    }
}

void RenderWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton && _render)
    {
        RenderCommand cmd;
        cmd.type = RenderCommand::Type::DeselectNode;
        _render->submitCommand(std::move(cmd));
    }
}

void RenderWidget::wheelEvent(QWheelEvent *event)
{
    if (_render)
    {
        RenderCommand cmd;
        cmd.type = RenderCommand::Type::ZoomCamera;
        cmd.zoomFactor = event->angleDelta().y() / 120.0f; // 每格=1，正值放大
        _render->submitCommand(std::move(cmd));
    }
    event->accept();
}