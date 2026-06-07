#pragma once

#include <QRect>
#include <QVector>

class QWidget;

namespace markshot::windows {

QVector<QRect> enumerateWindowGeometries();
void setExcludedFromCapture(QWidget *widget, bool excluded = true);

} // namespace markshot::windows
