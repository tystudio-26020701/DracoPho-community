#pragma once

#include <QRect>
#include <QString>
#include <QVector>

namespace markshot {

QVector<QRect> collectConfiguredWindowGeometries(const QRect &captureGeometry,
                                                 const QString &outputName,
                                                 bool allOutputs);

} // namespace markshot
