#include "RenderWidget.h"
#include "Render.h"
#include "Node.h"
#include "Texture.h"

#include <QThread>
#include <QPainter>
#include <QMouseEvent>
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
    if (event->button() == Qt::RightButton && _render)
    {
        _lastMousePos = event->pos();
        RenderCommand cmd;
        cmd.type = RenderCommand::Type::SelectNode;
        cmd.pixelX = event->pos().x();
        cmd.pixelY = event->pos().y();
        _render->submitCommand(std::move(cmd));
    }
}

void RenderWidget::mouseMoveEvent(QMouseEvent *event)
{
    // 预留：后续可在此处添加拖动逻辑
    Q_UNUSED(event);
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