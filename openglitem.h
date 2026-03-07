#ifndef OPENGLITEM_H
#define OPENGLITEM_H

#include <QQuickFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QMatrix4x4>
#include <QStringList>

class OpenGLRenderer;

// ─────────────────────────────────────────────────────────────────
// OpenGLItem：QML 侧暴露的控件
// ─────────────────────────────────────────────────────────────────
class OpenGLItem : public QQuickFramebufferObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList activeFilters READ activeFilters WRITE setActiveFilters NOTIFY activeFiltersChanged)
    Q_PROPERTY(QString imagePath READ imagePath WRITE setImagePath NOTIFY imagePathChanged)

public:
    explicit OpenGLItem(QQuickItem *parent = nullptr);
    Renderer *createRenderer() const override;

    QStringList activeFilters() const { return m_activeFilters; }
    void setActiveFilters(const QStringList &filters);

    QString imagePath() const { return m_imagePath; }
    void setImagePath(const QString &path);

signals:
    void activeFiltersChanged();
    void imagePathChanged();

private:
    friend class OpenGLRenderer;
    QStringList m_activeFilters;
    QString     m_imagePath = QStringLiteral(":/images/lenna.png"); // 默认图片
};

// ─────────────────────────────────────────────────────────────────
// OpenGLRenderer：实际 GL 渲染逻辑
// ─────────────────────────────────────────────────────────────────
class OpenGLRenderer : public QQuickFramebufferObject::Renderer,
                       protected QOpenGLFunctions
{
public:
    OpenGLRenderer();
    ~OpenGLRenderer() override;

    void render() override;
    void synchronize(QQuickFramebufferObject *item) override;
    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override;

private:
    void initialize();
    void loadTexture(const QString &path);
    QByteArray buildShaderSource(const QString &qrcPath);

    QOpenGLShaderProgram     *m_program = nullptr;
    QOpenGLVertexArrayObject *m_vao     = nullptr;
    QOpenGLBuffer            *m_vbo     = nullptr;
    QOpenGLTexture           *m_texture = nullptr;

    // uniform locations
    int m_textureLoc    = -1;
    int m_filterModeLoc = -1;

    // 滤镜位掩码（与 fragment shader 中常量保持一致）
    enum FilterFlag {
        FILTER_NONE      = 0,
        FILTER_GRAYSCALE = 1 << 0,
        FILTER_INVERT    = 1 << 1,
        FILTER_BLUR      = 1 << 2,
        FILTER_SHARPEN   = 1 << 3,
        FILTER_EDGE      = 1 << 4,
        FILTER_WARM      = 1 << 5,
        FILTER_COOL      = 1 << 6,
        FILTER_SEPIA     = 1 << 7,
    };

    int     m_filterMask     = FILTER_NONE;
    QString m_pendingImgPath = QStringLiteral(":/images/lenna.png");
    QString m_loadedImgPath;
    bool    m_initialized    = false;
    bool    m_imgDirty       = true;   // 首次强制加载
};

#endif // OPENGLITEM_H
