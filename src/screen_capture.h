#pragma once

#include <QImage>
#include <QRect>
#include <QString>
#include <QVector>

struct CaptureResult {
    QImage image;
    QString error;
    QString outputName;
    QRect sourceGeometry;
};

struct CaptureRequest {
    QString preferredOutputName;
    QRect sourceGeometry;
    bool allOutputs = false;
};

CaptureResult captureScreenFrame(const CaptureRequest &request);
QVector<QRect> enumerateX11WindowGeometries();
