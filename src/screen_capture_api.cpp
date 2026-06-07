#include "screen_capture_internal.h"

CaptureResult captureScreenFrame(const CaptureRequest &request)
{
#ifdef MARK_SHOT_WITH_DBUS
    CaptureResult result = isWaylandSession()
        ? captureWaylandFrame(request)
        : captureWithQScreen(request);
#else
    CaptureResult result = captureWithQScreen(request);
#endif
    result.image = normalizeCaptureImage(result.image);
    return result;
}

void stopActiveScreencastCapture()
{
#ifdef MARK_SHOT_WITH_DBUS
    stopPortalScreencast();
#endif
}
