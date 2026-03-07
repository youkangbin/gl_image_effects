#include "openglitem.h"
#include <QOpenGLFramebufferObjectFormat>

// ─────────────────────────────────────────────
// OpenGLItem
// ─────────────────────────────────────────────
OpenGLItem::OpenGLItem(QQuickItem *parent)
    : QQuickFramebufferObject(parent)
{
    setMirrorVertically(true); // 修正 FBO → QML 的 Y 轴翻转
}

QQuickFramebufferObject::Renderer *OpenGLItem::createRenderer() const
{
    return new OpenGLRenderer();
}

// ─────────────────────────────────────────────
// OpenGLRenderer
// ─────────────────────────────────────────────
OpenGLRenderer::OpenGLRenderer() {}

OpenGLRenderer::~OpenGLRenderer()
{
    delete m_program;
    delete m_vao;
    delete m_vbo;
}

QOpenGLFramebufferObject *OpenGLRenderer::createFramebufferObject(const QSize &size)
{
    QOpenGLFramebufferObjectFormat fmt;
    fmt.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    fmt.setSamples(4);
    return new QOpenGLFramebufferObject(size, fmt);
}

// ─── 完整的立方体：6面 × 2三角形 × 3顶点 = 36顶点 ───────────────
// 每个顶点：x y z  u v
static const GLfloat cubeVertices[] = {
    // 前面 (z = +0.5)
    -0.5f,  0.5f,  0.5f,   0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,   0.0f, 0.0f,
    0.5f, -0.5f,  0.5f,   1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,   0.0f, 1.0f,
    0.5f, -0.5f,  0.5f,   1.0f, 0.0f,
    0.5f,  0.5f,  0.5f,   1.0f, 1.0f,
    // 后面 (z = -0.5)
    0.5f,  0.5f, -0.5f,   0.0f, 1.0f,
    0.5f, -0.5f, -0.5f,   0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,   1.0f, 0.0f,
    0.5f,  0.5f, -0.5f,   0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,   1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,   1.0f, 1.0f,
    // 左面 (x = -0.5)
    -0.5f,  0.5f, -0.5f,   0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,   0.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,   1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,   0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,   1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,   1.0f, 1.0f,
    // 右面 (x = +0.5)
    0.5f,  0.5f,  0.5f,   0.0f, 1.0f,
    0.5f, -0.5f,  0.5f,   0.0f, 0.0f,
    0.5f, -0.5f, -0.5f,   1.0f, 0.0f,
    0.5f,  0.5f,  0.5f,   0.0f, 1.0f,
    0.5f, -0.5f, -0.5f,   1.0f, 0.0f,
    0.5f,  0.5f, -0.5f,   1.0f, 1.0f,
    // 顶面 (y = +0.5)
    -0.5f,  0.5f, -0.5f,   0.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,   0.0f, 0.0f,
    0.5f,  0.5f,  0.5f,   1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,   0.0f, 1.0f,
    0.5f,  0.5f,  0.5f,   1.0f, 0.0f,
    0.5f,  0.5f, -0.5f,   1.0f, 1.0f,
    // 底面 (y = -0.5)
    -0.5f, -0.5f,  0.5f,   0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,   0.0f, 0.0f,
    0.5f, -0.5f, -0.5f,   1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,   0.0f, 1.0f,
    0.5f, -0.5f, -0.5f,   1.0f, 0.0f,
    0.5f, -0.5f,  0.5f,   1.0f, 1.0f,
};

void OpenGLRenderer::initialize()
{
    // 必须在 Renderer 线程上调用，此处 GL context 已激活
    initializeOpenGLFunctions();

    // ── 着色器 ──────────────────────────────────────
    m_program = new QOpenGLShaderProgram();

    // 兼容 OpenGL ES 2.0 / Desktop GL 的 GLSL 写法
    const char *vertSrc = R"(
        attribute highp vec4 vertex;
        attribute highp vec2 coord;
        varying   highp vec2 v_coord;
        uniform   highp mat4 matrix;
        void main() {
            v_coord     = coord;
            gl_Position = matrix * vertex;
        }
    )";

    const char *fragSrc = R"(
        varying highp vec2 v_coord;
        void main() {
            // 用 UV 坐标生成彩色渐变，方便验证每个面都渲染正确
            gl_FragColor = vec4(v_coord.x, v_coord.y, 0.6, 1.0);
        }
    )";

    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex,   vertSrc);
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragSrc);
    m_program->bindAttributeLocation("vertex", 0);
    m_program->bindAttributeLocation("coord",  1);
    if (!m_program->link()) {
        qWarning() << "Shader link error:" << m_program->log();
        return;
    }
    m_matrixLoc = m_program->uniformLocation("matrix");

    // ── VAO / VBO ───────────────────────────────────
    m_vao = new QOpenGLVertexArrayObject();
    m_vao->create();
    QOpenGLVertexArrayObject::Binder vaoBinder(m_vao);

    m_vbo = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    m_vbo->create();
    m_vbo->bind();
    m_vbo->allocate(cubeVertices, sizeof(cubeVertices));

    // 在 VAO 内记录属性指针
    m_program->bind();
    m_program->enableAttributeArray(0);
    m_program->enableAttributeArray(1);
    m_program->setAttributeBuffer(0, GL_FLOAT, 0,                  3, 5 * sizeof(GLfloat));
    m_program->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(GLfloat),2, 5 * sizeof(GLfloat));
    m_program->release();

    m_vbo->release();
    // vaoBinder 析构时自动解绑 VAO

    m_initialized = true;
}

void OpenGLRenderer::render()
{
    if (!m_initialized)
        initialize();

    // ── 投影矩阵（尺寸变化时重建） ─────────────────
    const QSize sz = framebufferObject()->size();
    m_proj.setToIdentity();
    m_proj.perspective(45.0f,
                       sz.width() / float(sz.height()),
                       0.01f, 100.0f);

    // ── 渲染状态 ────────────────────────────────────
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // ── 绘制 ─────────────────────────────────────────
    m_program->bind();
    {
        QOpenGLVertexArrayObject::Binder vaoBinder(m_vao);

        QMatrix4x4 model;
        model.translate(0.0f, 0.0f, -2.0f);
        model.rotate(m_angle,       0.5f, 1.0f, 0.0f);  // 绕斜轴旋转
        m_angle += 0.8f;                                  // 每帧角度增量

        m_program->setUniformValue(m_matrixLoc, m_proj * model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    m_program->release();

    // 告知 Qt 场景图本帧结束后继续请求下一帧 → 持续动画
    update();
}
