#include "scroll/stitcher_internal.h"

#include "debug_log.h"

#include <algorithm>
#include <cmath>
#include <cstdarg>
#include <limits>
#include <vector>

namespace markshot::scroll::stitcher_internal {


/// @brief Ratio of frame height to ignore at the top of the content field.
constexpr float kColTopIgnoreRatio = 0.10f;
/// @brief Ratio of frame height to ignore at the bottom of the content field.
constexpr float kColBottomIgnoreRatio = 0.08f;
/// @brief Minimum number of pixels to ignore at the top or bottom of a frame.
constexpr int kColMinIgnorePx = 16;
/// @brief Maximum number of sample columns to inspect per row comparison.
constexpr int kLineMaxSampleCols = 256;

/// @brief Calculates the number of ignore pixels based on height and ratio.
/// @param height The frame height.
/// @param ratio The ignore ratio.
/// @return The number of pixels to ignore.
int scaledIgnore(int height, float ratio)
{
    if (height < 80) {
        return 0;
    }
    return std::min(height / 4, std::max(kColMinIgnorePx, static_cast<int>(height * ratio)));
}

int contentTopIgnore(int height)
{
    return scaledIgnore(height, kColTopIgnoreRatio);
}

int contentBottomIgnore(int height)
{
    return scaledIgnore(height, kColBottomIgnoreRatio);
}

bool isContentRow(int y, int height)
{
    return y >= contentTopIgnore(height) && y < height - contentBottomIgnore(height);
}

bool shouldCropContentRows(int overlapLen, int frameHeight, int minOverlap)
{
    return overlapLen >= minOverlap + contentTopIgnore(frameHeight) + contentBottomIgnore(frameHeight);
}

int requiredComparedRows(int minOverlap, bool cropped)
{
    return cropped ? std::max(24, minOverlap / 2) : minOverlap;
}

std::pair<int, int> bandRange(int width, float startRatio, float endRatio)
{
    const int start = std::clamp(static_cast<int>(std::lround(width * startRatio)), 0, width - 1);
    const int end = std::clamp(static_cast<int>(std::lround(width * endRatio)), start, width - 1);
    return {start, end};
}

int overhangAmount(int pos, int frameHeight, int fullHeight, StitchEdge *edge)
{
    const int overTop = std::max(0, -pos);
    const int overBottom = std::max(0, pos + frameHeight - fullHeight);
    if (edge) {
        *edge = overBottom >= overTop ? StitchEdge::End : StitchEdge::Start;
    }
    return std::max(overTop, overBottom);
}

int sideIgnoreWidth(int width)
{
    if (width <= 0) {
        return 0;
    }
    const int wide = std::min(std::max(50, width / 20), width / 3);
    return std::min(wide, std::max(0, (width - 1) / 2));
}

float rowMeanAbsDiff(const QImage &a, int ay, const QImage &b, int by, int startX, int width)
{
    if (ay < 0 || ay >= a.height() || by < 0 || by >= b.height() || width <= 0) {
        return kNoMatchConfidence;
    }

    const QRgb *aLine = reinterpret_cast<const QRgb *>(a.constScanLine(ay));
    const QRgb *bLine = reinterpret_cast<const QRgb *>(b.constScanLine(by));
    const int step = std::max(1, width / kLineMaxSampleCols);
    float sum = 0.0f;
    int count = 0;
    for (int x = startX; x < startX + width; x += step) {
        const QRgb ap = aLine[x];
        const QRgb bp = bLine[x];
        sum += std::abs(qRed(ap) - qRed(bp));
        sum += std::abs(qGreen(ap) - qGreen(bp));
        sum += std::abs(qBlue(ap) - qBlue(bp));
        count += 3;
    }
    return count > 0 ? sum / static_cast<float>(count) : kNoMatchConfidence;
}

// Mean absolute difference of the overlapping region when cols2 (current frame)
// is shifted down by `offset` relative to cols1 (previous frame). Low-value top
// and bottom rows are ignored only when enough overlap remains.
/// @brief Computes the difference metric between column vectors of two frames.
/// @param cols1 The column pixel values of the first frame.
/// @param cols2 The column pixel values of the second frame.
/// @param offset The shift offset of the second frame relative to the first.
/// @param minOverlap The minimum required overlap between the frames.
/// @return The computed column difference score.
float computeColDiff(const QVector<std::array<float, 3>> &cols1,
                     const QVector<std::array<float, 3>> &cols2,
                     int offset,
                     int minOverlap)
{
    const int h1 = static_cast<int>(cols1.size());
    const int h2 = static_cast<int>(cols2.size());
    if (h1 == 0 || h2 == 0) {
        return kNoMatchConfidence;
    }

    const int overlapLen = offset >= 0
        ? std::min(h1 - offset, h2)
        : std::min(h1, h2 + offset);
    if (overlapLen < minOverlap) {
        return kNoMatchConfidence;
    }

    const bool cropRows = shouldCropContentRows(overlapLen, std::min(h1, h2), minOverlap);
    float sum = 0.0f;
    int count = 0;
    int rows = 0;

    if (offset >= 0) {
        const int len = std::min(h1 - offset, h2);
        for (int i = 0; i < len; ++i) {
            const int y1 = offset + i;
            const int y2 = i;
            if (cropRows && (!isContentRow(y1, h1) || !isContentRow(y2, h2))) {
                continue;
            }
            ++rows;
            for (int g = 0; g < 3; ++g) {
                sum += std::abs(cols1[y1][g] - cols2[y2][g]);
                ++count;
            }
        }
    } else {
        const int o = -offset;
        const int len = std::min(h1, h2 - o);
        for (int i = 0; i < len; ++i) {
            const int y1 = i;
            const int y2 = o + i;
            if (cropRows && (!isContentRow(y1, h1) || !isContentRow(y2, h2))) {
                continue;
            }
            ++rows;
            for (int g = 0; g < 3; ++g) {
                sum += std::abs(cols1[y1][g] - cols2[y2][g]);
                ++count;
            }
        }
    }

    if (count == 0 || rows < requiredComparedRows(minOverlap, cropRows)) {
        return kNoMatchConfidence;
    }
    return sum / static_cast<float>(count);
}

// Search order centred on the predicted offset, expanding outward:
// [p, p+1, p-1, p+2, p-2, ...], clamped to [-max, +max]. Signed so reverse
// scrolling (negative offsets) is searched too: the previous offset carries its
// sign as the prediction centre, giving reverse momentum just like forward.
/// @brief Generates search offsets centered on a predicted value.
/// @param max The maximum search offset (defines search range [-max, max]).
/// @param predict The predicted search center offset.
/// @return A vector of integer offsets to search.
std::vector<int> predictOffsetIter(int max, int predict)
{
    const int m = std::max(0, max);
    const int p = std::clamp(predict, -m, m);
    std::vector<int> result;
    result.reserve(static_cast<std::size_t>(m) * 2 + 1);
    result.push_back(p);
    for (int delta = 1; delta <= m; ++delta) {
        if (p + delta <= m) {
            result.push_back(p + delta);
        }
        if (p - delta >= -m) {
            result.push_back(p - delta);
        }
    }
    return result;
}

// Swaps the X and Y axes of an image: (x, y) -> (y, x). Self-inverse. Used to
// run the entire vertical stitching pipeline on horizontally-scrolled frames by
// transposing on the way in and transposing the accumulated result on the way
// out.
QImage transposeImage(const QImage &src)
{
    if (src.isNull()) {
        return src;
    }
    const QImage rgb = src.format() == QImage::Format_ARGB32_Premultiplied
        ? src
        : src.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    const int w = rgb.width();
    const int h = rgb.height();
    QImage dst(h, w, QImage::Format_ARGB32_Premultiplied);
    for (int y = 0; y < h; ++y) {
        const QRgb *srcLine = reinterpret_cast<const QRgb *>(rgb.scanLine(y));
        for (int x = 0; x < w; ++x) {
            reinterpret_cast<QRgb *>(dst.scanLine(x))[y] = srcLine[x];
        }
    }
    return dst;
}

QImage normalizePixelImage(QImage image)
{
    if (!image.isNull()) {
        if (image.format() != QImage::Format_ARGB32_Premultiplied) {
            image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
        }
        image.setDevicePixelRatio(1.0);
    }
    return image;
}

const char *algorithmDebugName()
{
    return "col-sample";
}

const char *axisDebugName(ScrollAxis axis)
{
    return axis == ScrollAxis::Horizontal ? "horizontal" : "vertical";
}

const char *edgeDebugName(StitchEdge edge)
{
    switch (edge) {
    case StitchEdge::Start:
        return "start";
    case StitchEdge::End:
        return "end";
    case StitchEdge::None:
        return "none";
    }
    return "unknown";
}

void logStitchDebug(const char *format, ...)
{
    if (!markshot::debugEnabled()) {
        return;
    }
    va_list args;
    va_start(args, format);
    markshot::debugLogV("stitch", format, args);
    va_end(args);
}


}  // namespace markshot::scroll::stitcher_internal
