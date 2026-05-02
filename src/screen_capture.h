#pragma once

#include <QImage>
#include <QString>

struct CaptureResult {
    QImage image;
    QString error;
    QString outputName;
};

CaptureResult captureScreenFrame(const QString &preferredOutputName, bool allOutputs);
