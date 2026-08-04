#pragma once

#include "recording/recording_options.h"

#include <QString>
#include <QVector>

namespace markshot::recording {

/**
 * 读取当前可用于录制的显示器来源。
 * @return 显示器来源列表，多屏时包含全部显示器来源。
 */
QVector<DisplaySource> availableDisplaySources();

/**
 * 把用户提供的显示器标识归一化为录制持久化键（recordingDisplayPersistenceKey
 * 的格式）。录制持久化键对每个真实显示器几乎总是 "output:<name>"，因此裸屏名
 * （"DP-1"）与旧式 "screen:<name>" 都必须映射为 "output:<name>"；"output:"、
 * "geometry:"、"all" 原样返回。
 * @param raw 原始标识（可含空白）。
 * @return 归一化后的键；空输入返回空串。
 */
QString normalizeRecordingDisplayId(const QString &raw);

}  // namespace markshot::recording
