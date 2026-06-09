#pragma once

#include <QImage>
#include <QRect>
#include <QSize>

namespace markshot::capture {

QRect scaledCropRect(QRect sourceGeometry, QRect requestedGeometry, QSize imageSize);
QImage cropFrameToRequest(const QImage &frame, QRect streamGeometry, QRect requestedGeometry);
/// @brief 将捕获帧缩放到指定几何的逻辑尺寸。
/// @param frame 原始捕获帧。
/// @param geometry 目标逻辑几何。
/// @return 尺寸与目标几何一致的捕获帧。
QImage resizeFrameToGeometrySize(const QImage &frame, QRect geometry);
QRect imageRectFromGeometry(QRect geometry, QRect sourceGeometry, QSize imageSize);
QRect geometryFromImageRect(QRect imageRect, QRect sourceGeometry, QSize imageSize);

} // namespace markshot::capture
