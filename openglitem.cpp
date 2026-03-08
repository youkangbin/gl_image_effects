#include "openglitem.h"
#include <QOpenGLFramebufferObjectFormat>
#include <QOpenGLContext>
#include <QFile>
#include <QImage>
#include <QDebug>

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
    if (filters.contains("grayscale"))
        mask |= FILTER_GRAYSCALE;
    if (filters.contains("invert"))
        mask |= FILTER_INVERT;
    if (filters.contains("blur"))
        mask |= FILTER_BLUR;
    if (filters.contains("sharpen"))
        mask |= FILTER_SHARPEN;
    if (filters.contains("edge"))
        mask |= FILTER_EDGE;
    if (filters.contains("warm"))
        mask |= FILTER_WARM;
    if (filters.contains("cool"))
        mask |= FILTER_COOL;
    if (filters.contains("sepia"))
        mask |= FILTER_SEPIA;
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
    QString convertedPath;
    QUrl urlPath(path);
    convertedPath = urlPath.isValid() ? urlPath.toLocalFile() : path;

    delete m_texture;
    m_texture = nullptr;

    QImage img(convertedPath);
    if (img.isNull()) {
        qWarning() << "Failed to load image:" << convertedPath;
        img = QImage(2, 2, QImage::Format_RGBA8888);
        img.setPixel(0, 0, qRgba(255,   0, 255, 255));
        img.setPixel(1, 0, qRgba( 64,  64,  64, 255));
        img.setPixel(0, 1, qRgba( 64,  64,  64, 255));
        img.setPixel(1, 1, qRgba(255,   0, 255, 255));
    }

    // ── 人脸检测（在 mirrored 之前，用原始坐标系检测）──────────
    m_faceData = m_faceDetector.detectFaces(img);
    m_imgSize  = img.size();          // 记录图片尺寸，供坐标映射用
    m_faceDataDirty = true;
    qDebug() << "Detected faces:" << m_faceData.size();

    // 转为 RGBA8888 并上下翻转（OpenGL 纹理原点在左下）
    img = img.convertToFormat(QImage::Format_RGBA8888).mirrored();

    m_texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
    m_texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    m_texture->setMagnificationFilter(QOpenGLTexture::Linear);
    m_texture->setWrapMode(QOpenGLTexture::ClampToEdge);
    m_texture->setData(img);
}

// ── 初始化 GL 资源 ────────────────────────────────────────────
void OpenGLRenderer::initialize()
{
    initializeOpenGLFunctions();

    // ── 着色器 ──
    m_program = new QOpenGLShaderProgram();

    QByteArray vertSrc = buildShaderSource(":/shaders/filter.vert");
    QByteArray fragSrc = buildShaderSource(":/shaders/filter.frag");

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


    auto ret = m_faceDetector.loadLandmarkModelFromFile(":/models/shape_predictor_81_face_landmarks.dat");
    if (ret != true) {
        qWarning("face detecotr unable load model!");
    }

    initPointRenderer();

    m_initialized = true;
}

// ── 主渲染函数 ────────────────────────────────────────────────
void OpenGLRenderer::render()
{
    if (!m_initialized)
        initialize();

    if (m_imgDirty) {
        loadTexture(m_pendingImgPath);
        m_loadedImgPath = m_pendingImgPath;
        m_imgDirty = false;
    }

    // ── 每次图片更新时重新上传关键点 ──
    if (m_faceDataDirty)
        uploadLandmarkPoints();

    glDisable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (!m_texture) return;

    // ① 绘制图片（含滤镜）
    m_program->bind();
    {
        QOpenGLVertexArrayObject::Binder vaoBinder(m_vao);
        m_texture->bind(0);
        m_program->setUniformValue(m_textureLoc,    0);
        m_program->setUniformValue(m_filterModeLoc, m_filterMask);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        m_texture->release();
    }
    m_program->release();

    // ② 叠加绘制关键点
    renderLandmarks();
}

void OpenGLRenderer::renderLandmarks()
{
    if (m_pointCount <= 0 || !m_pointProgram) return;

    glEnable(GL_PROGRAM_POINT_SIZE);   // 让 gl_PointSize 生效

    m_pointProgram->bind();
    m_pointProgram->setUniformValue(m_pointColorLoc,
                                    QColor(0, 255, 0, 230)); // 亮绿色

    QOpenGLVertexArrayObject::Binder vaoBinder(m_pointVao);
    glDrawArrays(GL_POINTS, 0, m_pointCount);

    m_pointProgram->release();
    glDisable(GL_PROGRAM_POINT_SIZE);
}

void OpenGLRenderer::uploadLandmarkPoints()
{
    if (!m_faceDataDirty || m_faceData.empty()) {
        m_pointCount = 0;
        m_faceDataDirty = false;
        return;
    }

    std::vector<float> pts;
    const float W = static_cast<float>(m_imgSize.width());
    const float H = static_cast<float>(m_imgSize.height());

    for (const FaceData &fd : m_faceData) {
        if (!fd.hasShape) continue;
        for (unsigned long i = 0; i < fd.shape.num_parts(); ++i) {
            auto &p = fd.shape.part(i);
            // dlib 坐标：(0,0) 在左上，y 向下
            // OpenGL NDC：(0,0) 在中心，y 向上
            // 注意：纹理已经 mirrored，所以 y 轴需要翻转
            float nx =  (static_cast<float>(p.x()) / W) * 2.0f - 1.0f;
            float ny = -(static_cast<float>(p.y()) / H) * 2.0f + 1.0f; // 翻转 y
            pts.push_back(nx);
            pts.push_back(ny);
        }
    }

    m_pointCount = static_cast<int>(pts.size() / 2);

    QOpenGLVertexArrayObject::Binder vaoBinder(m_pointVao);
    m_pointVbo->bind();
    m_pointVbo->allocate(pts.data(), static_cast<int>(pts.size() * sizeof(float)));

    m_pointProgram->bind();
    m_pointProgram->enableAttributeArray(0);
    m_pointProgram->setAttributeBuffer(0, GL_FLOAT, 0, 2, 2 * sizeof(float));
    m_pointProgram->release();
    m_pointVbo->release();

    m_faceDataDirty = false;
}

void OpenGLRenderer::initPointRenderer()
{
    // 极简顶点着色器：直接接受 NDC 坐标
    const char *vertSrc = R"(
    layout(location = 0) in vec2 a_pos;
    void main() {
        gl_Position  = vec4(a_pos, 0.0, 1.0);
        gl_PointSize = 20.0;          // 关键点圆点大小（像素）
    }
    )";

    const char *fragSrc = R"(
    uniform vec4 u_color;
    out vec4 fragColor;
    void main() {
        // 画圆点（丢弃圆外像素）
        vec2 c = gl_PointCoord - vec2(0.5);
        if (dot(c, c) > 0.25) discard;
        fragColor = u_color;
    }
    )";

    m_pointProgram = new QOpenGLShaderProgram();
    m_pointProgram->addShaderFromSourceCode(QOpenGLShader::Vertex,   vertSrc);
    m_pointProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, fragSrc);
    m_pointProgram->bindAttributeLocation("a_pos", 0);
    m_pointProgram->link();
    m_pointColorLoc = m_pointProgram->uniformLocation("u_color");

    m_pointVao = new QOpenGLVertexArrayObject();
    m_pointVao->create();

    m_pointVbo = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    m_pointVbo->create();
    m_pointVbo->setUsagePattern(QOpenGLBuffer::DynamicDraw); // 每帧可更新
}
