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
        QImage::Format_ARGB32
    );

    QPainter painter(this);    
    painter.drawImage(rect(), img);
}

void RenderWidget::mousePressEvent(QMouseEvent *event)
{
    return;
    _lastMousePos = event->pos();
}

void RenderWidget::mouseMoveEvent(QMouseEvent *event)
{
    return;
    qDebug() << _lastMousePos << event->pos();

    QPoint delta = event->pos() - _lastMousePos;
    _lastMousePos = event->pos();

    // 根据偏移量更新相机角度
    //_renderer.camera.rotate(delta.x(), delta.y());

    update();
}