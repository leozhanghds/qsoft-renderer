#include "RenderWidget.h"
#include "Render.h"
#include "Node.h"
#include "Texture.h"

#include <QThread>
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <QApplication>

RenderWidget::RenderWidget() : QWidget(nullptr)
{}

RenderWidget::~RenderWidget()
{}

void RenderWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    if (!_render) return;

    _render->clear(CLEAR_COLOR_BUFFER | CLEAR_DEPTH_BUFFER);
    _render->draw();

    QImage img(
        (uchar*)_render->getFrameBuffer(),
        _render->_width,
        _render->_height,
        QImage::Format_ARGB32
    );

    QPainter p(this);
    p.drawImage(rect(), img);

    update();
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