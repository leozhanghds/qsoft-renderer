#include "RenderWidget.h"
#include "Render.h"
#include "Node.h"
#include "Texture.h"

#include <QThread>
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <QApplication>

RenderWidget::RenderWidget(std::unique_ptr<DoubleBuffer>& buffer) : QWidget(nullptr), _doubleBuffer(buffer)
{
    _renderTimer = new QTimer(this);
    connect(_renderTimer, &QTimer::timeout, this, [this]() {
        _render->renderOneFrame();
        this->update();
    });
    _renderTimer->start(16); // 60 FPS
}

RenderWidget::~RenderWidget()
{}

void RenderWidget::paintEvent(QPaintEvent *event)
{
    // if (!_doubleBuffer->hasNewFrame())
    //     return;

    Q_UNUSED(event);

    const auto& buffer = _doubleBuffer->front();

    QImage img(
        (uchar*)buffer.pixels.data(),
        buffer.width,
        buffer.height,
        QImage::Format_ARGB32
    );

    //img.save("debug.png");

    QPainter p(this);
    p.drawImage(rect(), img);

    update();

    //_doubleBuffer->consumeFrame();
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