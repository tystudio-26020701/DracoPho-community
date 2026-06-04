#pragma once

#include "layer_shell_plugin_interface.h"

namespace markshot::layershell {

bool configureOverlay(QWidget *widget, QScreen *screen, const OverlayConfig &config);

} // namespace markshot::layershell
