#ifndef OPENGLITEM_H
#define OPENGLITEM_H

#include <QQuickFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLFunctions>
#include <QMatrix4x4>

class OpenGLRenderer;

class OpenGLItem : public QQuickFramebufferObject
{
    Q_OBJECT
    QML_ELEMENT
public:
    explicit OpenGLItem(QQuickItem *parent = nullptr);
    Renderer *createRenderer() const override;

private:
    friend class OpenGLRenderer;
};

class OpenGLRenderer : public QQuickFramebufferObject::Renderer,
                       protected QOpenGLFunctions
{
public:
    OpenGLRenderer();
    ~OpenGLRenderer() override;

    void render() override;
    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override;

private:
    void initialize();

    QOpenGLShaderProgram *m_program = nullptr;
    QOpenGLVertexArrayObject *m_vao  = nullptr;
    QOpenGLBuffer            *m_vbo  = nullptr;

    QMatrix4x4 m_proj;
    int   m_matrixLoc  = -1;
    float m_angle      = 0.0f;
    bool  m_initialized = false;
};

#endif // OPENGLITEM_H
