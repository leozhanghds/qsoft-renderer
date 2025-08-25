#pragma once

#include <memory>

#include <QWidget>
#include <QImage>

class Render;

class RenderWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RenderWidget(QWidget* parent = nullptr);
    ~RenderWidget();

    void render();

protected:
    void paintEvent(QPaintEvent* event) override;
        
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    QPoint _lastMousePos;
    QImage _displayImage;  

    std::unique_ptr<Render> _render;
};