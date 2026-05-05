#pragma once

#include <QImage>
#include <QRect>
#include <QString>

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
