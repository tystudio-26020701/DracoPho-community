#include "screen_capture_internal.h"

CaptureResult captureScreenFrame(const CaptureRequest &request)
{
    // The public API hides platform selection from callers. Wayland needs portal,
    // PipeWire, grim, or compositor helpers; non-Wayland sessions can use Qt's
    // QScreen grab path directly.
#ifdef MARK_SHOT_WITH_DBUS
    CaptureResult result = isWaylandSession()
        ? captureWaylandFrame(request)
        : captureWithQScreen(request);
#else
    CaptureResult result = captureWithQScreen(request);
#endif
    // Downstream selection, annotation, and stitching code works in raw image
    // pixels, so normalize format/device-pixel-ratio at the backend boundary.
    result.image = normalizeCaptureImage(result.image);
    return result;
}

void stopActiveScreencastCapture()
{
#ifdef MARK_SHOT_WITH_DBUS
    stopPortalScreencast();
#endif
}
