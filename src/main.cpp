#include <iostream>

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#endif

#include <QApplication>
#include "RenderWidget.h"

int main(int argc, char *argv[])
{
#if defined(_WIN32) || defined(_WIN64)
//#ifdef _DEBUG
	std::system("C:\\Windows\\System32\\chcp.com 65001");
	
	// 解除windwos下终端不敲回车不运行的bug
	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	DWORD mode;
	GetConsoleMode(hStdin, &mode);
	mode &= ~ENABLE_QUICK_EDIT_MODE; //移除快速编辑模式
	mode &= ~ENABLE_INSERT_MODE; //移除插入模式
	mode &= ~ENABLE_MOUSE_INPUT; //移除鼠标输入模式
	SetConsoleMode(hStdin, mode);
//#endif
#else
	// Linux 支持高DPI
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
	QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
	//QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

    QApplication app(argc, argv);

    RenderWidget renderWidget;
    renderWidget.show();

    renderWidget.render();

    return app.exec();
}