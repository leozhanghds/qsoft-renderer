#pragma once

#include <memory>

#include <QWidget>
#include <QImage>

#include "IRenderView.h"

#include "render_export.h"

class Render;

class RENDER_EXPORT RenderWidget : public IRenderView, public QWidget
{
    //Q_OBJECT
public:
    explicit RenderWidget();
    virtual ~RenderWidget();

protected:
    void paintEvent(QPaintEvent* event) override;
        
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    QPoint _lastMousePos;
    QImage _displayImage;  
};