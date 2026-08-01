#pragma once

#include <QColor>
#include <QImage>
#include <QJsonObject>

namespace markshot {

struct ExportImageEffectConfig {
    bool enabled = false;
    int padding = 112;
    qreal cornerRadius = 18.0;
    int shadowRadius = 72;
    int shadowOffsetY = 28;
    qreal shadowOpacity = 0.32;
    QColor shadowColor = QColor(0, 0, 0);
};

inline bool operator==(const ExportImageEffectConfig &a, const ExportImageEffectConfig &b)
{
    return a.enabled == b.enabled && a.padding == b.padding
        && a.cornerRadius == b.cornerRadius && a.shadowRadius == b.shadowRadius
        && a.shadowOffsetY == b.shadowOffsetY && a.shadowOpacity == b.shadowOpacity
        && a.shadowColor == b.shadowColor;
}

ExportImageEffectConfig exportImageEffectConfigFromRoot(const QJsonObject &root);
QImage applyExportImageEffect(const QImage &source, const ExportImageEffectConfig &config);

}  // namespace markshot
