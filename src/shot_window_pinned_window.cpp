#include "pinned_window/pinned_image_window.h"

namespace markshot::shot {

QWidget *createPinnedImageWindow(QImage image)
{
    return new PinnedImageWindow(std::move(image));
}

}  // namespace markshot::shot
