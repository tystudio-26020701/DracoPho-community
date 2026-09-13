#pragma once

#include "recording/recording_frame_sample.h"

#include <QImage>
#include <QSize>
#include <QString>

namespace markshot::recording {

class RecordingWriter {
public:
    virtual ~RecordingWriter() = default;

    /**
     * 启动写出器。
     * @param frameSize 录制帧尺寸。
     * @param fps 帧率。
     * @param error 输出错误信息。
     * @return 启动成功时返回 true。
     */
    virtual bool start(QSize frameSize, int fps, QString *error) = 0;

    /**
     * 写入一帧图像。
     * @param frame 需要写入的图像。
     * @param error 输出错误信息。
     * @return 写入成功时返回 true。
     */
    bool writeFrame(const QImage &frame, QString *error)
    {
        RecordingFrameSample sample;
        sample.image = frame;
        return writeFrame(sample, error);
    }

    /**
     * 写入带时间戳的帧样本。
     * @param sample 录制帧样本。
     * @param error 输出错误信息。
     * @return 写入成功时返回 true。
     */
    virtual bool writeFrame(const RecordingFrameSample &sample, QString *error) = 0;

    /**
     * 完成写出并关闭文件。
     * @param error 输出错误信息。
     * @return 完成成功时返回 true。
     */
    virtual bool finish(QString *error) = 0;

    /**
     * 取消写出并终止底层进程。
     * @return 无返回值。
     */
    virtual void cancel() = 0;

    /**
     * 编码器实际写入的帧数（含补帧），finish() 后调用才有精确值。
     * 控制器节流后的提交数可能少于实际编码帧——写线程按目标 fps 补帧
     * 填充时间线。JSON 报表应以本值为准。
     * @return 编码帧数。
     */
    virtual int writtenFrames() const { return 0; }
};

}  // namespace markshot::recording
