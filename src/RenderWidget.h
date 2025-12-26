#pragma once

#include <memory>

#include <QWidget>
#include <QImage>
#include <QTimer>

#include "IRenderView.h"

#include "render_export.h"

class Render;

class RENDER_EXPORT RenderWidget : public QWidget, public IRenderView
{
    Q_OBJECT
public:
    explicit RenderWidget(std::unique_ptr<DoubleBuffer>& buffer);
    virtual ~RenderWidget();

protected:
    void paintEvent(QPaintEvent* event) override;
        
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    QPoint _lastMousePos;
    QImage _displayImage;  

    QTimer* _renderTimer{nullptr};
};