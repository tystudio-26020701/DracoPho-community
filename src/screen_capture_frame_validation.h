#pragma once

#include <QImage>

namespace markshot {

/// @brief 判定捕获帧是否为"可疑纯色帧"：整帧降采样后颜色几乎一致。
///
/// 合成器在截图时机早于渲染完成、或区域覆盖未提交内容时，可能返回一整块
/// 纯色（如纯蓝）的占位帧，而不是真实画面；这类帧会被回退链当作"成功"
/// 结果直接返回，用户因此得到纯色图片。本函数用于把这类无效帧识别出来，
/// 让回退链继续尝试下一个后端。仅用于单帧截图回退链；滚动/录制等流式路径
/// （画面可能真的长时间单色）不适用。
bool isSuspiciousSolidFrame(const QImage &image);

}  // namespace markshot
