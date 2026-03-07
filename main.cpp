#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>      // ← 新增
#include "openglitem.h"

int main(int argc, char *argv[])
{
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

    QApplication app(argc, argv);

    qmlRegisterType<OpenGLItem>("MyComponents", 1, 0, "OpenGLItem");

    QQmlApplicationEngine engine;
    engine.addImportPath(":/imports");   // ← 保留原有的，让 Theme 模块可用
    engine.load(QUrl(QStringLiteral("qrc:/application.qml")));

    return app.exec();
}
