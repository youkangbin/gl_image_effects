#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QSurfaceFormat>
#include "openglitem.h"

int main(int argc, char *argv[])
{
    // 设置默认格式（Qt 6.5.3 推荐）
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
    QApplication app(argc, argv);

    qmlRegisterType<OpenGLItem>("MyComponents", 1, 0, "OpenGLItem");

    QQmlApplicationEngine engine;
    engine.addImportPath(":/imports");
    engine.load(QUrl(QStringLiteral("qrc:/application.qml")));

    return app.exec();
}
