#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#pragma once

#include <QMainWindow>
#include <QPushButton>

#include <memory>

class Node;
class Render;
class Texture;
class RenderWidget;

class MainWindow : public QMainWindow
{
public:
    MainWindow(QWidget* parent = nullptr);
    virtual ~MainWindow();

    void addCubeLayer();
    void deleteCubeLayer();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    QPushButton* addCubeButton{nullptr};
    QPushButton* deleteCubeButton{nullptr};
    
    std::unique_ptr<Render> _render{nullptr};
    std::unique_ptr<RenderWidget> _renderWidget{nullptr};

    std::shared_ptr<Texture> _sharedTexture{nullptr};
    std::shared_ptr<Node> _cubeNode{nullptr};
};

#endif