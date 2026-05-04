#pragma once

#include "shot_window.h"

#include <QIcon>
#include <QString>

namespace markshot::ui {

// Human-readable label for a toolbar action (used in tooltips and the
// runtime tool-name display).
QString actionName(ShotWindow::Action action);

// Generates a 32x32 pixmap-backed QIcon for the given action. All icons are
// drawn with QPainter so the binary stays free of image assets.
QIcon makeToolIcon(ShotWindow::Action action);

// Hollow square when filled=false, solid square when filled=true. Used by the
// Fill toggle so the glyph itself communicates the state.
QIcon makeFillIcon(bool filled);

}  // namespace markshot::ui
