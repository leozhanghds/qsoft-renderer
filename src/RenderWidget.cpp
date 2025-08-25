#include "RenderWidget.h"
#include "Render.h"

#include <QThread>
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <QApplication>

#define WIDGET_WIDTH 800
#define WIDGET_HEIGHT 800

RenderWidget::RenderWidget(QWidget *parent)
    :QWidget(parent)
{
    _render = std::make_unique<Render>(WIDGET_WIDTH, WIDGET_HEIGHT);
    _displayImage = QImage((uchar*)_render->getFrameBuffer(),_render->_width, _render->_height, QImage::Format_ARGB32);
    
    setWindowTitle("RenderWidget");
    setFixedSize(_render->_width, _render->_height);
}

RenderWidget::~RenderWidget()
{
}

void RenderWidget::render()
{
    while(true){
        QApplication::processEvents();// 处理ui事件
        _render->frame();
        update();

        break;

        QThread::msleep(20);
    }
}

void RenderWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.drawImage(0, 0, _displayImage);
}

void RenderWidget::mousePressEvent(QMouseEvent* event) {
    _lastMousePos = event->pos();
}

void RenderWidget::mouseMoveEvent(QMouseEvent* event) {

    qDebug() << _lastMousePos << event->pos();

    QPoint delta = event->pos() - _lastMousePos;
    _lastMousePos = event->pos();

    // 根据偏移量更新相机角度
    //_renderer.camera.rotate(delta.x(), delta.y());
    
    update();
}