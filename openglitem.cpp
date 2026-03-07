#include "openglitem.h"
#include <QOpenGLFramebufferObjectFormat>
#include <QOpenGLContext>
#include <QFile>
#include <QImage>

// ════════════════════════════════════════════════════════════════
// 全屏四边形顶点（两个三角形覆盖整个 NDC [-1,1]）
// 格式：x  y   u  v
// ════════════════════════════════════════════════════════════════
static const GLfloat quadVertices[] = {
    // 三角形 1
    -1.0f,  1.0f,   0.0f, 1.0f,
    -1.0f, -1.0f,   0.0f, 0.0f,
     1.0f, -1.0f,   1.0f, 0.0f,
    // 三角形 2
    -1.0f,  1.0f,   0.0f, 1.0f,
     1.0f, -1.0f,   1.0f, 0.0f,
     1.0f,  1.0f,   1.0f, 1.0f,
};

// ════════════════════════════════════════════════════════════════
// OpenGLItem
// ════════════════════════════════════════════════════════════════
OpenGLItem::OpenGLItem(QQuickItem *parent)
    : QQuickFramebufferObject(parent)
{
    setMirrorVertically(true);
}

QQuickFramebufferObject::Renderer *OpenGLItem::createRenderer() const
{
    return new OpenGLRenderer();
}

void OpenGLItem::setActiveFilters(const QStringList &filters)
{
    if (m_activeFilters == filters) return;
    m_activeFilters = filters;
    emit activeFiltersChanged();
    update();   // 通知场景图重新渲染
}

void OpenGLItem::setImagePath(const QString &path)
{
    if (m_imagePath == path) return;
    m_imagePath = path;
    emit imagePathChanged();
    update();
}

// ════════════════════════════════════════════════════════════════
// OpenGLRenderer
// ════════════════════════════════════════════════════════════════
OpenGLRenderer::OpenGLRenderer() {}

OpenGLRenderer::~OpenGLRenderer()
{
    delete m_program;
    delete m_vao;
    delete m_vbo;
    delete m_texture;
}

QOpenGLFramebufferObject *OpenGLRenderer::createFramebufferObject(const QSize &size)
{
    QOpenGLFramebufferObjectFormat fmt;
    fmt.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    fmt.setSamples(4);
    return new QOpenGLFramebufferObject(size, fmt);
}

// ── synchronize：在渲染线程上从 Item 同步数据 ──────────────────
void OpenGLRenderer::synchronize(QQuickFramebufferObject *item)
{
    auto *glItem = static_cast<OpenGLItem *>(item);

    // 同步图片路径
    if (glItem->m_imagePath != m_pendingImgPath) {
        m_pendingImgPath = glItem->m_imagePath;
        m_imgDirty = true;
    }

    // 把 QML 字符串列表转成位掩码
    const QStringList &filters = glItem->m_activeFilters;
    int mask = FILTER_NONE;
    if (filters.contains("grayscale")) mask |= FILTER_GRAYSCALE;
    if (filters.contains("invert"))    mask |= FILTER_INVERT;
    if (filters.contains("blur"))      mask |= FILTER_BLUR;
    if (filters.contains("sharpen"))   mask |= FILTER_SHARPEN;
    if (filters.contains("edge"))      mask |= FILTER_EDGE;
    if (filters.contains("warm"))      mask |= FILTER_WARM;
    if (filters.contains("cool"))      mask |= FILTER_COOL;
    if (filters.contains("sepia"))     mask |= FILTER_SEPIA;
    m_filterMask = mask;
}

QByteArray OpenGLRenderer::buildShaderSource(const QString &qrcPath)
{
    // 根据当前上下文判断是否是 ES
    const bool isES = QOpenGLContext::currentContext()->isOpenGLES();
    QByteArray header;
    if (isES) {
        header = "#version 320 es\nprecision highp float;\nprecision highp sampler2D;\n";
    } else {
        header = "#version 330 core\n";
    }

    QFile f(qrcPath);
    if (!f.open(QFile::ReadOnly)) {
        qWarning() << "Cannot open shader file:" << qrcPath;
        return {};
    }
    return header + f.readAll();
}

// ── 加载纹理 ───────────────────────────────────────────────────
void OpenGLRenderer::loadTexture(const QString &path)
{
    delete m_texture;
    m_texture = nullptr;

    QImage img(path);
    if (img.isNull()) {
        qWarning() << "Failed to load image:" << path;
        // 生成一个 2×2 的棋盘格作为 fallback
        img = QImage(2, 2, QImage::Format_RGBA8888);
        img.setPixel(0, 0, qRgba(255,   0, 255, 255));
        img.setPixel(1, 0, qRgba( 64,  64,  64, 255));
        img.setPixel(0, 1, qRgba( 64,  64,  64, 255));
        img.setPixel(1, 1, qRgba(255,   0, 255, 255));
    }

    // 转为 RGBA8888 并上下翻转（OpenGL 纹理原点在左下）
    img = img.convertToFormat(QImage::Format_RGBA8888).mirrored();

    m_texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
    m_texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    m_texture->setMagnificationFilter(QOpenGLTexture::Linear);
    m_texture->setWrapMode(QOpenGLTexture::ClampToEdge);
    m_texture->setData(img);   // 自动生成 mipmap
}

// ── 初始化 GL 资源 ────────────────────────────────────────────
void OpenGLRenderer::initialize()
{
    initializeOpenGLFunctions();

    // ── 着色器 ──
    m_program = new QOpenGLShaderProgram();

    QByteArray vertSrc = buildShaderSource(":/shader/filter.vert");
    QByteArray fragSrc = buildShaderSource(":/shader/filter.frag");

    if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex,   vertSrc) ||
        !m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragSrc)) {
        qWarning() << "Shader compile error:" << m_program->log();
        return;
    }
    m_program->bindAttributeLocation("a_position", 0);
    m_program->bindAttributeLocation("a_texCoord", 1);
    if (!m_program->link()) {
        qWarning() << "Shader link error:" << m_program->log();
        return;
    }

    m_textureLoc    = m_program->uniformLocation("u_texture");
    m_filterModeLoc = m_program->uniformLocation("u_filterMode");

    // ── 全屏四边形 VAO/VBO ──
    m_vao = new QOpenGLVertexArrayObject();
    m_vao->create();
    QOpenGLVertexArrayObject::Binder vaoBinder(m_vao);

    m_vbo = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    m_vbo->create();
    m_vbo->bind();
    m_vbo->allocate(quadVertices, sizeof(quadVertices));

    m_program->bind();
    // a_position: vec2 @ offset 0,  stride 4*float
    m_program->enableAttributeArray(0);
    m_program->setAttributeBuffer(0, GL_FLOAT, 0,                  2, 4 * sizeof(GLfloat));
    // a_texCoord: vec2 @ offset 2*float
    m_program->enableAttributeArray(1);
    m_program->setAttributeBuffer(1, GL_FLOAT, 2 * sizeof(GLfloat),2, 4 * sizeof(GLfloat));
    m_program->release();
    m_vbo->release();

    m_initialized = true;
}

// ── 主渲染函数 ────────────────────────────────────────────────
void OpenGLRenderer::render()
{
    if (!m_initialized)
        initialize();

    // 按需加载 / 更新纹理
    if (m_imgDirty) {
        loadTexture(m_pendingImgPath);
        m_loadedImgPath = m_pendingImgPath;
        m_imgDirty = false;
    }

    glDisable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (!m_texture) return;

    m_program->bind();
    {
        QOpenGLVertexArrayObject::Binder vaoBinder(m_vao);

        // 绑定纹理到 unit 0
        m_texture->bind(0);
        m_program->setUniformValue(m_textureLoc,    0);
        m_program->setUniformValue(m_filterModeLoc, m_filterMask);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        m_texture->release();
    }
    m_program->release();

    // 图片处理不需要持续动画，无需 update()
    // 如需实时预览动态效果可在此调用 update()
}
