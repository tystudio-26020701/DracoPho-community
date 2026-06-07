#pragma once

#include "scroll/stitcher.h"

#include <QImage>
#include <QVector>

#include <array>
#include <utility>
#include <vector>

namespace markshot::scroll::stitcher_internal {

inline constexpr int kColMaxBandSamples = 17;
inline constexpr float kLineRowMaxDiff = 2.0f;

int contentTopIgnore(int height);
int contentBottomIgnore(int height);
bool isContentRow(int y, int height);
bool shouldCropContentRows(int overlapLen, int frameHeight, int minOverlap);
int requiredComparedRows(int minOverlap, bool cropped);
std::pair<int, int> bandRange(int width, float startRatio, float endRatio);
int overhangAmount(int pos, int frameHeight, int fullHeight, StitchEdge *edge);
int sideIgnoreWidth(int width);
float rowMeanAbsDiff(const QImage &a, int ay, const QImage &b, int by, int startX, int width);
float computeColDiff(const QVector<std::array<float, 3>> &cols1,
                     const QVector<std::array<float, 3>> &cols2,
                     int offset,
                     int minOverlap);
std::vector<int> predictOffsetIter(int max, int predict);
QImage transposeImage(const QImage &src);
QImage normalizePixelImage(QImage image);
const char *algorithmDebugName();
const char *axisDebugName(ScrollAxis axis);
const char *edgeDebugName(StitchEdge edge);
void logStitchDebug(const char *format, ...);

}  // namespace markshot::scroll::stitcher_internal
