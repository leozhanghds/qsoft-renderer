#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#pragma once

#include <QMainWindow>
#include <QPushButton>

#include <memory>

class Node;
class Render;
class Texture;
class DoubleBuffer;
class RenderWidget;
class RenderThread;

class MainWindow : public QMainWindow
{
public:
    MainWindow(QWidget* parent = nullptr);
    virtual ~MainWindow();

    void addCubeLayer();
    void deleteCubeLayer();

    void addBunnyLayer();
    void deleteBunnyLayer();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    QPushButton* addCubeButton{nullptr};
    QPushButton* deleteCubeButton{nullptr};
    QPushButton* addBunnyButton{nullptr};
    QPushButton* deleteBunnyButton{nullptr};
    
    std::unique_ptr<Render> _render{nullptr};
    std::unique_ptr<RenderWidget> _renderWidget{nullptr};
    std::unique_ptr<DoubleBuffer> _renderBuffer{nullptr};
    std::unique_ptr<RenderThread> _renderThread{nullptr};

    std::shared_ptr<Texture> _sharedTexture{nullptr};
    std::shared_ptr<Node> _cubeNode{nullptr};
    std::shared_ptr<Node> _bunnyNode{nullptr};
};

#endif