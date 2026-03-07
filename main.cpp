#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>      // ← 新增
#include "openglitem.h"

int main(int argc, char *argv[])
{
    QSurfaceFormat fmt;
    fmt.setRenderableType(QSurfaceFormat::OpenGLES); // 明确使用 GLES
    fmt.setVersion(3, 2);                            // ES 3.2
    fmt.setDepthBufferSize(24);
    fmt.setSamples(4);
    fmt.setProfile(QSurfaceFormat::CoreProfile);     // ES 核心配置
    QSurfaceFormat::setDefaultFormat(fmt);

    // ========== 关键修复3：正确设置 QQuickWindow 图形 API ==========
    // QSGRendererInterface::OpenGLES 而非 OpenGL
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGLES);


    QApplication app(argc, argv);

    qmlRegisterType<OpenGLItem>("MyComponents", 1, 0, "OpenGLItem");

    QQmlApplicationEngine engine;
    engine.addImportPath(":/imports");   // ← 保留原有的，让 Theme 模块可用
    engine.load(QUrl(QStringLiteral("qrc:/application.qml")));

    return app.exec();
}
