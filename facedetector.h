#ifndef FACEDETECTOR_H
#define FACEDETECTOR_H

#include <QImage>
#include <QDebug>
#include <QFile>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing.h>
#include <dlib/image_io.h>

// 人脸检测结果结构体，包含人脸矩形框和关键点信息
struct FaceData {
    dlib::rectangle             rect;          // 人脸矩形区域
    dlib::full_object_detection shape;         // 人脸关键点
    bool                        hasShape = false; // 是否检测到关键点
};

class FaceDetector {
public:
    // 构造函数：加载人脸关键点模型
    FaceDetector() {
        // 初始化人脸检测器
        detector = dlib::get_frontal_face_detector();
        hasLandmarks = false;
    }

    // 从内存加载关键点模型（兼容原代码的qrc资源加载逻辑）
    bool loadLandmarkModelFromMemory(const QByteArray& modelData) {
        if (modelData.isEmpty()) {
            qCritical() << "关键点模型数据为空";
            return false;
        }

        try {
            std::vector<char> buffer(modelData.constData(),
                                     modelData.constData() + modelData.size());
            dlib::vectorstream vs(buffer);
            dlib::deserialize(poseModel, vs);
            hasLandmarks = true;
            qInfo() << "成功加载人脸关键点模型";
            return true;
        } catch (const std::exception& e) {
            hasLandmarks = false;
            qCritical() << "加载dlib模型失败：" << e.what();
            return false;
        }
    }

    // 从文件加载关键点模型（补充接口，方便使用）
    bool loadLandmarkModelFromFile(const QString& filePath) {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            qCritical() << "无法打开模型文件：" << filePath;
            return false;
        }

        QByteArray byteData = file.readAll();
        file.close();
        return loadLandmarkModelFromMemory(byteData);
    }

    // 核心接口：检测人脸并返回关键点信息
    std::vector<FaceData> detectFaces(const QImage &origImg) {
        if (origImg.isNull()) {
            qWarning() << "输入图片为空，无法检测人脸";
            return {};
        }

        const int origW = origImg.width();
        const int origH = origImg.height();

        // 缩放图片以提高检测速度（最大尺寸限制）
        static constexpr int DETECT_MAX_DIM = 480;
        double scale = 1.0;
        QImage detectImg = origImg;

        if (qMax(origW, origH) > DETECT_MAX_DIM) {
            scale = static_cast<double>(DETECT_MAX_DIM) / qMax(origW, origH);
            detectImg = origImg.scaled(
                qRound(origW * scale), qRound(origH * scale),
                Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }

        // QImage转dlib格式
        dlib::array2d<dlib::rgb_pixel> dlibDetect;
        qimage_to_dlib(detectImg, dlibDetect);

        // 转灰度图用于快速检测
        dlib::array2d<unsigned char> grayDetect;
        rgb_to_gray(dlibDetect, grayDetect);

        // 检测人脸
        std::vector<dlib::rectangle> rawFaces = detector(grayDetect, 0);

        // 处理检测结果（还原缩放比例）
        std::vector<FaceData> results;
        for (const auto& faceSmall : rawFaces) {
            FaceData fd;
            // 还原人脸矩形框到原始图片尺寸
            fd.rect = dlib::rectangle(
                qRound(faceSmall.left()   / scale),
                qRound(faceSmall.top()    / scale),
                qRound(faceSmall.right()  / scale),
                qRound(faceSmall.bottom() / scale));

            // 如果加载了关键点模型，检测人脸关键点
            if (hasLandmarks) {
                dlib::full_object_detection shapeSmall =
                    poseModel(dlibDetect, faceSmall);

                // 还原关键点到原始图片尺寸
                std::vector<dlib::point> parts;
                parts.reserve(shapeSmall.num_parts());
                for (unsigned i = 0; i < shapeSmall.num_parts(); ++i) {
                    parts.emplace_back(
                        qRound(shapeSmall.part(i).x() / scale),
                        qRound(shapeSmall.part(i).y() / scale));
                }
                fd.shape    = dlib::full_object_detection(fd.rect, parts);
                fd.hasShape = true;
            }
            results.push_back(fd);
        }

        return results;
    }

    // 获取模型加载状态
    bool isModelLoaded() const {
        return hasLandmarks;
    }

private:
    // QImage(RGB888) → dlib rgb格式转换
    static void qimage_to_dlib(const QImage& qimg,
                               dlib::array2d<dlib::rgb_pixel>& out) {
        QImage src = qimg.convertToFormat(QImage::Format_RGB888);
        out.set_size(src.height(), src.width());
        for (int r = 0; r < src.height(); ++r) {
            const uchar* line = src.constScanLine(r);
            for (int c = 0; c < src.width(); ++c) {
                out[r][c].red   = line[c * 3 + 0];
                out[r][c].green = line[c * 3 + 1];
                out[r][c].blue  = line[c * 3 + 2];
            }
        }
    }

    // dlib rgb → 灰度图转换
    static void rgb_to_gray(const dlib::array2d<dlib::rgb_pixel>& src,
                            dlib::array2d<unsigned char>& dst) {
        dst.set_size(src.nr(), src.nc());
        for (long r = 0; r < src.nr(); ++r)
            for (long c = 0; c < src.nc(); ++c) {
                const auto& p = src[r][c];
                dst[r][c] = static_cast<unsigned char>(
                    0.299 * p.red + 0.587 * p.green + 0.114 * p.blue);
            }
    }

    // 成员变量
    dlib::frontal_face_detector detector;  // 人脸检测器
    dlib::shape_predictor       poseModel; // 关键点预测器
    bool                        hasLandmarks = false; // 关键点模型是否加载成功
};

#endif // FACEDETECTOR_H
