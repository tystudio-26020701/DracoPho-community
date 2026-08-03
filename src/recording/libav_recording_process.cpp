#include "recording/libav_recording_process.h"

#include "recording/audio/audio_capture_reader.h"
#include "recording/audio/audio_capture_reader_factory.h"
#include "recording/libav_audio_encoder.h"
#include "recording/libav_error.h"
#include "recording/recording_frame_converter.h"

#include <QByteArray>
#include <QDateTime>
#include <QFile>

#include <algorithm>
#include <atomic>
#include <memory>
#include <mutex>

#ifdef HAVE_LIBAV_RECORDING
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
#include <libswscale/swscale.h>
}
#else
struct AVFrame;
#endif

namespace markshot::recording {
namespace {

/**
 * 向调用方写入错误文本。
 * @param error 输出错误信息。
 * @param text 错误文本。
 * @return 固定返回 false，便于调用处直接返回。
 */
bool failWith(QString *error, const QString &text)
{
    if (error) {
        *error = text;
    }
    return false;
}

/**
 * 把尺寸压到 yuv420p 可接受的偶数宽高。
 * @param size 输入帧尺寸。
 * @return 编码尺寸。
 */
QSize evenEncodedSize(QSize size)
{
    return {std::max(2, size.width() & ~1), std::max(2, size.height() & ~1)};
}

#ifdef HAVE_LIBAV_RECORDING

/**
 * 【录制】【库内编码】读取编码器支持的像素格式列表。
 * @param codec 目标编码器。
 * @return 像素格式数组，编码器未声明时返回空指针。
 */
const enum AVPixelFormat *encoderPixelFormats(const AVCodec *codec)
{
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(61, 13, 100)
    const enum AVPixelFormat *formats = nullptr;
    int count = 0;
    if (avcodec_get_supported_config(nullptr,
                                     codec,
                                     AV_CODEC_CONFIG_PIX_FORMAT,
                                     0,
                                     reinterpret_cast<const void **>(&formats),
                                     &count) < 0) {
        return nullptr;
    }
    return formats;
#else
    return codec->pix_fmts;
#endif
}

/**
 * 【录制】【库内编码】为编码器选择系统内存输入像素格式。
 * @param codec 目标编码器。
 * @return 优先 yuv420p、其次 nv12，再退到首个非硬件格式。
 */
AVPixelFormat chooseEncoderPixelFormat(const AVCodec *codec)
{
    const enum AVPixelFormat *formats = encoderPixelFormats(codec);
    if (!formats) {
        return AV_PIX_FMT_YUV420P;
    }

    AVPixelFormat firstSoftware = AV_PIX_FMT_NONE;
    bool hasYuv420p = false;
    bool hasNv12 = false;
    for (const enum AVPixelFormat *format = formats; *format != AV_PIX_FMT_NONE; ++format) {
        const AVPixFmtDescriptor *descriptor = av_pix_fmt_desc_get(*format);
        if (descriptor && (descriptor->flags & AV_PIX_FMT_FLAG_HWACCEL)) {
            continue;
        }
        if (*format == AV_PIX_FMT_YUV420P) {
            hasYuv420p = true;
        }
        if (*format == AV_PIX_FMT_NV12) {
            hasNv12 = true;
        }
        if (firstSoftware == AV_PIX_FMT_NONE) {
            firstSoftware = *format;
        }
    }
    if (hasYuv420p) {
        return AV_PIX_FMT_YUV420P;
    }
    if (hasNv12) {
        return AV_PIX_FMT_NV12;
    }
    return firstSoftware != AV_PIX_FMT_NONE ? firstSoftware : AV_PIX_FMT_YUV420P;
}

/**
 * 【录制】【库内编码】估算硬件编码器目标码率。
 * @param size 编码尺寸。
 * @param fps 目标帧率。
 * @return 码率（bit/s）。
 */
qint64 estimatedHardwareBitRate(QSize size, int fps)
{
    const qint64 pixelRate = static_cast<qint64>(size.width()) * size.height() * std::max(1, fps);
    return std::max<qint64>(1000000, pixelRate / 10);
}

#endif

}  // namespace

class LibavRecordingProcessPrivate final {
public:
    /**
     * 启动库内 FFmpeg 编码器。
     * @param options 录制配置。
     * @param encoder 编码器候选。
     * @param frameSize 输入帧尺寸。
     * @param fps 目标帧率。
     * @param error 输出错误信息。
     * @return 启动成功时返回 true。
     */
    bool start(const RecordingOptions &options,
               const RecordingVideoEncoderOptions &encoder,
               QSize frameSize,
               int fps,
               QString *error);

    /**
     * 写入一帧录制样本。
     * @param sample 录制帧样本。
     * @param error 输出错误信息。
     * @return 写入成功时返回 true。
     */
    bool writeFrame(const RecordingFrameSample &sample, QString *error);

    /**
     * 复用上一帧已转换数据补写一帧，跳过像素转换。
     * @param error 输出错误信息。
     * @return 写入成功时返回 true。
     */
    bool writeRepeatFrame(QString *error);

    /**
     * 冲刷编码器并关闭输出文件。
     * @param error 输出错误信息。
     * @return 完成成功时返回 true。
     */
    bool finish(QString *error);

    /**
     * 取消编码并释放资源。
     * @return 无返回值。
     */
    void cancel();

private:
    /**
     * 初始化输出容器。
     * @param outputPath 输出路径。
     * @param error 输出错误信息。
     * @return 初始化成功时返回 true。
     */
    bool openOutput(const QString &outputPath, QString *error);

    /**
     * 结束时把临时 MKV 流拷贝 remux 为最终 MP4（崩溃安全路径）。
     * @param tempMkvPath 临时 MKV 路径。
     * @param error 输出错误信息。
     * @return 成功时返回 true。
     */
    bool remuxTempToFinal(const QString &tempMkvPath, QString *error);

    /**
     * 初始化视频编码器。
     * @param encoder 编码器候选。
     * @param fps 目标帧率。
     * @param error 输出错误信息。
     * @return 初始化成功时返回 true。
     */
    bool openEncoder(const RecordingVideoEncoderOptions &encoder, int fps, QString *error);

    /**
     * 初始化音频编码器和采集器。
     * @param error 输出错误信息。
     * @return 初始化成功时返回 true。
     */
    bool openAudio(QString *error);

    /**
     * 首帧视频到达时启动音频采集。
     * @return 无返回值。
     */
    void startAudioCaptureIfNeeded();

    /**
     * 编码音频采集线程送来的 PCM 样本。
     * @param sample 音频样本。
     * @return 无返回值。
     */
    void encodeAudioSample(const AudioCaptureSample &sample);

    /**
     * 把录制样本转换为连续 BGRA 字节。
     * @param sample 录制帧样本。
     * @param error 输出错误信息。
     * @return BGRA 字节视图。
     */
    RecordingBgraFrame bgraBytesForSample(const RecordingFrameSample &sample, QString *error);

    /**
     * 把 BGRA 输入帧转换为编码帧。
     * @param bytes BGRA 字节视图。
     * @param error 输出错误信息。
     * @return 转换成功时返回 true。
     */
    bool fillVideoFrame(RecordingBgraFrame bytes, QString *error);

    /**
     * 把帧送入编码器并写出已生成的 packet。
     * @param frame 输入帧，传空表示冲刷编码器。
     * @param error 输出错误信息。
     * @return 写出成功时返回 true。
     */
    bool encodeFrame(AVFrame *frame, QString *error);

    /**
     * 释放所有 FFmpeg 资源。
     * @return 无返回值。
     */
    void cleanup();

#ifdef HAVE_LIBAV_RECORDING
    AVFormatContext *m_formatContext = nullptr;
    AVDictionary *m_formatOptions = nullptr;
    AVCodecContext *m_codecContext = nullptr;
    AVStream *m_stream = nullptr;
    AVFrame *m_frame = nullptr;
    AVPacket *m_packet = nullptr;
    SwsContext *m_swsContext = nullptr;
    AVPixelFormat m_encodePixelFormat = AV_PIX_FMT_YUV420P;
#endif
    RecordingFrameConverter m_converter;
    LibavAudioEncoder m_audioEncoder;
    std::unique_ptr<AudioCaptureReader> m_audioReader;
    QByteArray m_outputPathBytes;
    // 崩溃安全：录制期间写入临时 MKV（增量落盘），结束时机内 remux 成最终
    // MP4。进程被杀死时临时 MKV 仍可恢复，避免直接写 MP4 全量丢失。
    QString m_tempMkvPath;
    QString m_finalOutputPath;
    QSize m_frameSize;
    QSize m_encodedSize;
    std::mutex m_writeMutex;
    std::atomic<bool> m_audioFailed{false};
    int m_fps = 30;
    int64_t m_nextPts = 0;
    bool m_started = false;
    bool m_enableAudio = false;
    bool m_audioCaptureStarted = false;
};

bool LibavRecordingProcessPrivate::start(const RecordingOptions &options,
                                         const RecordingVideoEncoderOptions &encoder,
                                         QSize frameSize,
                                         int fps,
                                         QString *error)
{
    if (error) {
        error->clear();
    }
#ifndef HAVE_LIBAV_RECORDING
    Q_UNUSED(options)
    Q_UNUSED(encoder)
    Q_UNUSED(frameSize)
    Q_UNUSED(fps)
    return failWith(error, QStringLiteral("FFmpeg libraries are not linked"));
#else
    if (frameSize.isEmpty()) {
        return failWith(error, QStringLiteral("Cannot start libav writer with an empty frame size"));
    }

    cleanup();
    // 上一次录制若在 remux 前中断（进程被杀），会遗留 .part.mkv；新录制开始时
    // 清掉旧残留，避免占用磁盘。cancel() 与正常 finish 已各自清理。
    if (!m_tempMkvPath.isEmpty()) {
        QFile::remove(m_tempMkvPath);
        m_tempMkvPath.clear();
    }
    m_frameSize = frameSize;
    m_encodedSize = evenEncodedSize(frameSize);
    m_fps = std::max(1, fps);
    m_nextPts = 0;
    m_enableAudio = options.includeAudio;
    m_audioFailed = false;
    m_audioCaptureStarted = false;

    if (!openOutput(options.outputPath, error) || !openEncoder(encoder, m_fps, error)) {
        cleanup();
        return false;
    }
    if (m_enableAudio && !openAudio(error)) {
        cleanup();
        return false;
    }

    int result = avformat_write_header(m_formatContext, &m_formatOptions);
    av_dict_free(&m_formatOptions);
    m_formatOptions = nullptr;
    if (result < 0) {
        cleanup();
        return failWith(error,
                        QStringLiteral("Failed to write libav output header: %1")
                            .arg(libavErrorText(result)));
    }
    m_started = true;
    return true;
#endif
}

bool LibavRecordingProcessPrivate::writeFrame(const RecordingFrameSample &sample, QString *error)
{
    if (error) {
        error->clear();
    }
#ifndef HAVE_LIBAV_RECORDING
    Q_UNUSED(sample)
    return failWith(error, QStringLiteral("FFmpeg libraries are not linked"));
#else
    if (!m_started) {
        return failWith(error, QStringLiteral("libav writer is not started"));
    }
    if (m_audioFailed) {
        return failWith(error, QStringLiteral("libav audio capture or encoding failed"));
    }
    startAudioCaptureIfNeeded();

    const RecordingBgraFrame bytes = bgraBytesForSample(sample, error);
    if (!bytes.data || bytes.size <= 0) {
        return false;
    }
    if (!fillVideoFrame(bytes, error)) {
        return false;
    }
    m_frame->pts = m_nextPts++;
    return encodeFrame(m_frame, error);
#endif
}

bool LibavRecordingProcessPrivate::writeRepeatFrame(QString *error)
{
    if (error) {
        error->clear();
    }
#ifndef HAVE_LIBAV_RECORDING
    return failWith(error, QStringLiteral("FFmpeg libraries are not linked"));
#else
    if (!m_started) {
        return failWith(error, QStringLiteral("libav writer is not started"));
    }
    if (m_nextPts <= 0) {
        return failWith(error, QStringLiteral("libav writer has no previous frame to repeat"));
    }
    // 补帧复用 m_frame 中上一帧已转换像素，只推进 pts，避免重复 sws_scale
    m_frame->pts = m_nextPts++;
    return encodeFrame(m_frame, error);
#endif
}

bool LibavRecordingProcessPrivate::finish(QString *error)
{
    if (error) {
        error->clear();
    }
#ifndef HAVE_LIBAV_RECORDING
    return true;
#else
    if (!m_started) {
        cleanup();
        return true;
    }
    if (m_enableAudio) {
        if (m_audioReader) {
            m_audioReader->stop();
        }
        m_audioCaptureStarted = false;
    }
    if (!encodeFrame(nullptr, error)) {
        cleanup();
        return false;
    }
    if (m_enableAudio && !m_audioEncoder.flush(m_writeMutex, error)) {
        cleanup();
        return false;
    }
    const int result = av_write_trailer(m_formatContext);
    // cleanup 会删除临时文件；先取回路径供后续 remux 使用。
    const QString tempMkvPath = m_tempMkvPath;
    cleanup();
    if (result < 0) {
        return failWith(error,
                        QStringLiteral("Failed to write libav output trailer: %1")
                            .arg(libavErrorText(result)));
    }
    // 崩溃安全：把临时 MKV 流拷贝 remux 成最终 MP4（保留编码质量、避免重编码），
    // 成功后删除临时文件；失败时保留 .part.mkv 供恢复，并如实报告。
    if (!tempMkvPath.isEmpty()) {
        if (!remuxTempToFinal(tempMkvPath, error)) {
            return false;
        }
    }
    return true;
#endif
}

void LibavRecordingProcessPrivate::cancel()
{
    cleanup();
    if (!m_tempMkvPath.isEmpty()) {
        QFile::remove(m_tempMkvPath);
        m_tempMkvPath.clear();
    }
}

bool LibavRecordingProcessPrivate::remuxTempToFinal(const QString &tempMkvPath, QString *error)
{
#ifdef HAVE_LIBAV_RECORDING
    if (tempMkvPath.isEmpty() || m_finalOutputPath.isEmpty()) {
        return failWith(error, QStringLiteral("No temporary recording file to finalize"));
    }
    const QByteArray inputPathBytes = QFile::encodeName(tempMkvPath);
    const QByteArray outputPathBytes = QFile::encodeName(m_finalOutputPath);

    AVFormatContext *input = nullptr;
    AVFormatContext *output = nullptr;
    if (avformat_open_input(&input, inputPathBytes.constData(), nullptr, nullptr) < 0) {
        return failWith(error,
                        QStringLiteral("Failed to open temporary recording for finalization: %1")
                            .arg(tempMkvPath));
    }
    if (avformat_find_stream_info(input, nullptr) < 0) {
        avformat_close_input(&input);
        return failWith(error, QStringLiteral("Failed to read temporary recording stream info"));
    }

    int outputResult = avformat_alloc_output_context2(&output,
                                                      nullptr,
                                                      nullptr,
                                                      outputPathBytes.constData());
    if (outputResult < 0 || !output) {
        avformat_close_input(&input);
        return failWith(error,
                        QStringLiteral("Failed to allocate libav final output context: %1")
                            .arg(libavErrorText(outputResult)));
    }

    // 复制全部流（视频 + 音频），流拷贝保留编码质量。
    for (unsigned i = 0; i < input->nb_streams; ++i) {
        AVStream *inStream = input->streams[i];
        AVStream *outStream = avformat_new_stream(output, nullptr);
        if (!outStream) {
            avformat_close_input(&input);
            avformat_free_context(output);
            return failWith(error, QStringLiteral("Failed to create libav final output stream"));
        }
        if (avcodec_parameters_copy(outStream->codecpar, inStream->codecpar) < 0) {
            avformat_close_input(&input);
            avformat_free_context(output);
            return failWith(error, QStringLiteral("Failed to copy libav final stream parameters"));
        }
        outStream->time_base = inStream->time_base;
    }

    if (!(output->oformat->flags & AVFMT_NOFILE)) {
        outputResult = avio_open(&output->pb, outputPathBytes.constData(), AVIO_FLAG_WRITE);
        if (outputResult < 0) {
            avformat_close_input(&input);
            avformat_free_context(output);
            return failWith(error,
                            QStringLiteral("Failed to open libav final output file: %1")
                                .arg(libavErrorText(outputResult)));
        }
    }

    AVDictionary *finalOptions = nullptr;
    // faststart：把 moov 提到文件头，便于网络/流式场景即时播放。
    av_dict_set(&finalOptions, "movflags", "faststart", 0);
    outputResult = avformat_write_header(output, &finalOptions);
    av_dict_free(&finalOptions);
    if (outputResult < 0) {
        if (output->pb) {
            avio_closep(&output->pb);
        }
        avformat_close_input(&input);
        avformat_free_context(output);
        return failWith(error,
                        QStringLiteral("Failed to write libav final output header: %1")
                            .arg(libavErrorText(outputResult)));
    }

    AVPacket *packet = av_packet_alloc();
    if (!packet) {
        if (output->pb) {
            avio_closep(&output->pb);
        }
        avformat_close_input(&input);
        avformat_free_context(output);
        return failWith(error, QStringLiteral("Failed to allocate libav final packet"));
    }
    while (av_read_frame(input, packet) >= 0) {
        if (packet->stream_index >= 0
            && static_cast<unsigned>(packet->stream_index) < input->nb_streams) {
            AVStream *inStream = input->streams[packet->stream_index];
            AVStream *outStream = output->streams[packet->stream_index];
            av_packet_rescale_ts(packet, inStream->time_base, outStream->time_base);
        }
        packet->pos = -1;
        outputResult = av_interleaved_write_frame(output, packet);
        av_packet_unref(packet);
        if (outputResult < 0) {
            av_packet_free(&packet);
            if (output->pb) {
                avio_closep(&output->pb);
            }
            avformat_close_input(&input);
            avformat_free_context(output);
            return failWith(error,
                            QStringLiteral("Failed to finalize recording: %1")
                                .arg(libavErrorText(outputResult)));
        }
    }
    av_packet_free(&packet);
    outputResult = av_write_trailer(output);
    if (output->pb) {
        avio_closep(&output->pb);
    }
    avformat_close_input(&input);
    avformat_free_context(output);
    if (outputResult < 0) {
        return failWith(error,
                        QStringLiteral("Failed to write libav final output trailer: %1")
                            .arg(libavErrorText(outputResult)));
    }

    // remux 成功，删除临时 MKV。
    QFile::remove(tempMkvPath);
    m_tempMkvPath.clear();
    return true;
#else
    Q_UNUSED(error)
    return true;
#endif
}

bool LibavRecordingProcessPrivate::openAudio(QString *error)
{
#ifndef HAVE_LIBAV_RECORDING
    return failWith(error, QStringLiteral("FFmpeg libraries are not linked"));
#else
    m_audioReader = createPlatformAudioCaptureReader();
    if (!m_audioReader) {
        return failWith(error, recordingAudioUnavailableText());
    }
    const int audioSampleRate = m_audioReader->preferredSampleRate();
    if (!m_audioEncoder.open(m_formatContext, audioSampleRate, error)) {
        return false;
    }
    if (!m_audioReader->init(m_audioEncoder.frameBytes(),
                             audioSampleRate,
                             [this](const AudioCaptureSample &sample) {
                                 encodeAudioSample(sample);
                             },
                             error)) {
        return false;
    }
    return true;
#endif
}

void LibavRecordingProcessPrivate::startAudioCaptureIfNeeded()
{
    if (!m_enableAudio || m_audioCaptureStarted) {
        return;
    }
    m_audioCaptureStarted = true;
    if (m_audioReader) {
        m_audioReader->start();
    }
}

void LibavRecordingProcessPrivate::encodeAudioSample(const AudioCaptureSample &sample)
{
    QString error;
    if (!m_audioEncoder.encode(sample, m_writeMutex, &error)) {
        m_audioFailed = true;
    }
}

bool LibavRecordingProcessPrivate::openOutput(const QString &outputPath, QString *error)
{
#ifndef HAVE_LIBAV_RECORDING
    Q_UNUSED(outputPath)
    return failWith(error, QStringLiteral("FFmpeg libraries are not linked"));
#else
    m_finalOutputPath = outputPath;
    // 崩溃安全：MP4/MOV 直接写普通格式时 av_interleaved_write_frame 会把全部
    // 数据缓冲到 trailer，进程被杀时文件不可用（全量丢失）。改为录制期间写
    // 临时 MKV（增量落盘、中断可恢复），结束时 remux 成最终 MP4——与
    // vokoscreenNG/OBS 的可靠录制路径一致。
    m_tempMkvPath.clear();
    const QString lower = outputPath.toLower();
    if (lower.endsWith(QStringLiteral(".mp4")) || lower.endsWith(QStringLiteral(".mov"))) {
        // 随机后缀：避免可预测的 .part.mkv 被预置符号链接指向受害者文件
        // （共享/组可写输出目录下的经典符号链接攻击）。
        const QString randomSuffix =
            QString::number(QDateTime::currentMSecsSinceEpoch() ^ (quintptr(this) & 0xFFFF));
        m_tempMkvPath = QStringLiteral("%1.%2.part.mkv").arg(outputPath, randomSuffix);
    }
    const QByteArray muxPathBytes = QFile::encodeName(
        m_tempMkvPath.isEmpty() ? outputPath : m_tempMkvPath);
    m_outputPathBytes = muxPathBytes;
    int result = avformat_alloc_output_context2(&m_formatContext,
                                                nullptr,
                                                nullptr,
                                                muxPathBytes.constData());
    if (result < 0 || !m_formatContext) {
        return failWith(error,
                        QStringLiteral("Failed to allocate libav output context: %1")
                            .arg(libavErrorText(result)));
    }
    if (!(m_formatContext->oformat->flags & AVFMT_NOFILE)) {
        result = avio_open(&m_formatContext->pb, muxPathBytes.constData(), AVIO_FLAG_WRITE);
        if (result < 0) {
            return failWith(error,
                            QStringLiteral("Failed to open libav output file: %1")
                                .arg(libavErrorText(result)));
        }
    }
    // MKV 崩溃窗口：默认 cluster 每 5 秒落盘一次，中断最多丢最后 5 秒。
    // 缩短到 1 秒，把崩溃损失压到最小。
    if (!m_tempMkvPath.isEmpty()) {
        av_dict_set(&m_formatOptions, "cluster_time_limit", "1000000", 0);
    }
    return true;
#endif
}

bool LibavRecordingProcessPrivate::openEncoder(const RecordingVideoEncoderOptions &encoder,
                                               int fps,
                                               QString *error)
{
#ifndef HAVE_LIBAV_RECORDING
    Q_UNUSED(encoder)
    Q_UNUSED(fps)
    return failWith(error, QStringLiteral("FFmpeg libraries are not linked"));
#else
    const QByteArray encoderName = encoder.id.toUtf8();
    const AVCodec *codec = avcodec_find_encoder_by_name(encoderName.constData());
    if (!codec) {
        return failWith(error,
                        QStringLiteral("encoder %1 is not available in FFmpeg libraries")
                            .arg(encoder.id));
    }

    m_stream = avformat_new_stream(m_formatContext, codec);
    if (!m_stream) {
        return failWith(error, QStringLiteral("Failed to create libav video stream"));
    }

    m_codecContext = avcodec_alloc_context3(codec);
    if (!m_codecContext) {
        return failWith(error, QStringLiteral("Failed to allocate libav codec context"));
    }
    m_encodePixelFormat = chooseEncoderPixelFormat(codec);
    m_codecContext->width = m_encodedSize.width();
    m_codecContext->height = m_encodedSize.height();
    m_codecContext->pix_fmt = m_encodePixelFormat;
    m_codecContext->time_base = AVRational{1, fps};
    m_codecContext->framerate = AVRational{fps, 1};
    // 关键帧间隔 = 2 秒：便于拖动/剪辑定位，同时比逐秒关键帧节省码率
    // （SVT-AV1/x264 均建议 2*fps 左右；过密关键帧浪费码率，过疏难以跳转）。
    m_codecContext->gop_size = std::max(1, fps * 2);
    m_codecContext->max_b_frames = 0;
    // 0 表示自动按 CPU 核数分配编码线程，默认值 1 会让 libx264 单线程运行
    m_codecContext->thread_count = 0;
    if (m_formatContext->oformat->flags & AVFMT_GLOBALHEADER) {
        m_codecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    if (encoder.hardware) {
        m_codecContext->bit_rate = estimatedHardwareBitRate(m_encodedSize, fps);
    }

    AVDictionary *codecOptions = nullptr;
    if (encoder.id == QStringLiteral("libx264")) {
        av_dict_set(&codecOptions, "preset", fps >= 48 ? "ultrafast" : "veryfast", 0);
        av_dict_set(&codecOptions, "crf", "23", 0);
    }
    int result = avcodec_open2(m_codecContext, codec, &codecOptions);
    av_dict_free(&codecOptions);
    if (result < 0) {
        return failWith(error,
                        QStringLiteral("Failed to open libav video encoder %1: %2")
                            .arg(encoder.id, libavErrorText(result)));
    }

    result = avcodec_parameters_from_context(m_stream->codecpar, m_codecContext);
    if (result < 0) {
        return failWith(error,
                        QStringLiteral("Failed to copy libav codec parameters: %1")
                            .arg(libavErrorText(result)));
    }
    m_stream->time_base = m_codecContext->time_base;

    m_frame = av_frame_alloc();
    m_packet = av_packet_alloc();
    if (!m_frame || !m_packet) {
        return failWith(error, QStringLiteral("Failed to allocate libav frame or packet"));
    }
    m_frame->format = m_codecContext->pix_fmt;
    m_frame->width = m_codecContext->width;
    m_frame->height = m_codecContext->height;
    result = av_frame_get_buffer(m_frame, 32);
    if (result < 0) {
        return failWith(error,
                        QStringLiteral("Failed to allocate libav frame buffer: %1")
                            .arg(libavErrorText(result)));
    }

    m_swsContext = sws_getContext(m_frameSize.width(),
                                  m_frameSize.height(),
                                  AV_PIX_FMT_BGRA,
                                  m_encodedSize.width(),
                                  m_encodedSize.height(),
                                  m_encodePixelFormat,
                                  SWS_FAST_BILINEAR,
                                  nullptr,
                                  nullptr,
                                  nullptr);
    if (!m_swsContext) {
        return failWith(error, QStringLiteral("Failed to create libav scale context"));
    }
    return true;
#endif
}

RecordingBgraFrame LibavRecordingProcessPrivate::bgraBytesForSample(const RecordingFrameSample &sample,
                                                                    QString *error)
{
    if (sample.bgra.isValid() && sample.bgra.size == m_frameSize) {
        return {sample.bgra.constData(),
                sample.bgra.byteSize(),
                sample.bgra.stride,
                sample.bgra.yInverted};
    }
    if (!sample.image.isNull()) {
        return m_converter.convertToBgra(sample.image, m_frameSize, error);
    }
    failWith(error, QStringLiteral("Cannot write an empty recording frame"));
    return {};
}

bool LibavRecordingProcessPrivate::fillVideoFrame(RecordingBgraFrame bytes, QString *error)
{
#ifndef HAVE_LIBAV_RECORDING
    Q_UNUSED(bytes)
    return failWith(error, QStringLiteral("FFmpeg libraries are not linked"));
#else
    const int result = av_frame_make_writable(m_frame);
    if (result < 0) {
        return failWith(error,
                        QStringLiteral("Failed to make libav frame writable: %1")
                            .arg(libavErrorText(result)));
    }
    const char *source = bytes.data;
    int sourceStride = bytes.stride > 0 ? bytes.stride : m_frameSize.width() * 4;
    if (bytes.yInverted) {
        source += static_cast<qsizetype>(sourceStride) * (m_frameSize.height() - 1);
        sourceStride = -sourceStride;
    }
    const uint8_t *sourceData[] = {reinterpret_cast<const uint8_t *>(source)};
    const int sourceLineSize[] = {sourceStride};
    sws_scale(m_swsContext,
              sourceData,
              sourceLineSize,
              0,
              m_frameSize.height(),
              m_frame->data,
              m_frame->linesize);
    return true;
#endif
}

bool LibavRecordingProcessPrivate::encodeFrame(AVFrame *frame, QString *error)
{
#ifndef HAVE_LIBAV_RECORDING
    Q_UNUSED(frame)
    return failWith(error, QStringLiteral("FFmpeg libraries are not linked"));
#else
    int result = avcodec_send_frame(m_codecContext, frame);
    if (result < 0) {
        return failWith(error,
                        QStringLiteral("Failed to send frame to libav encoder: %1")
                            .arg(libavErrorText(result)));
    }

    while (result >= 0) {
        result = avcodec_receive_packet(m_codecContext, m_packet);
        if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) {
            return true;
        }
        if (result < 0) {
            return failWith(error,
                            QStringLiteral("Failed to receive packet from libav encoder: %1")
                                .arg(libavErrorText(result)));
        }
        av_packet_rescale_ts(m_packet, m_codecContext->time_base, m_stream->time_base);
        m_packet->stream_index = m_stream->index;
        {
            std::lock_guard<std::mutex> lock(m_writeMutex);
            // av_write_frame：立即把 packet 交给 muxer（MKV 按 cluster 增量
            // 写出）；av_interleaved_write_frame 会把全部数据缓冲到 trailer，
            // 崩溃即全量丢失。
            result = av_write_frame(m_formatContext, m_packet);
            if (result >= 0 && m_formatContext && m_formatContext->pb && !m_tempMkvPath.isEmpty()) {
                // 把 avio 缓冲立即刷到磁盘：默认 32KB 缓冲会吃掉小写入，
                // 不刷则进程被杀时仍有大量数据只停留在内存。
                avio_flush(m_formatContext->pb);
            }
        }
        av_packet_unref(m_packet);
        if (result < 0) {
            return failWith(error,
                            QStringLiteral("Failed to write libav packet: %1")
                                .arg(libavErrorText(result)));
        }
    }
    return true;
#endif
}

void LibavRecordingProcessPrivate::cleanup()
{
#ifdef HAVE_LIBAV_RECORDING
    if (m_audioReader) {
        m_audioReader->stop();
        m_audioReader.reset();
    }
    m_audioCaptureStarted = false;
    m_audioEncoder.close();
    if (m_codecContext && m_started) {
        avcodec_flush_buffers(m_codecContext);
    }
    if (m_frame) {
        av_frame_free(&m_frame);
    }
    if (m_packet) {
        av_packet_free(&m_packet);
    }
    if (m_swsContext) {
        sws_freeContext(m_swsContext);
        m_swsContext = nullptr;
    }
    if (m_codecContext) {
        avcodec_free_context(&m_codecContext);
    }
    if (m_formatContext) {
        if (m_formatContext->pb) {
            avio_closep(&m_formatContext->pb);
        }
        avformat_free_context(m_formatContext);
        m_formatContext = nullptr;
    }
    av_dict_free(&m_formatOptions);
    m_formatOptions = nullptr;
    m_stream = nullptr;
#endif
    m_started = false;
}

LibavRecordingProcess::LibavRecordingProcess()
    : m_impl(std::make_unique<LibavRecordingProcessPrivate>())
{
}

LibavRecordingProcess::~LibavRecordingProcess() = default;

bool LibavRecordingProcess::start(const RecordingOptions &options,
                                  const RecordingVideoEncoderOptions &encoder,
                                  QSize frameSize,
                                  int fps,
                                  QString *error)
{
    return m_impl->start(options, encoder, frameSize, fps, error);
}

bool LibavRecordingProcess::writeFrame(const RecordingFrameSample &sample, QString *error)
{
    return m_impl->writeFrame(sample, error);
}

bool LibavRecordingProcess::writeRepeatFrame(QString *error)
{
    return m_impl->writeRepeatFrame(error);
}

bool LibavRecordingProcess::finish(QString *error)
{
    return m_impl->finish(error);
}

void LibavRecordingProcess::cancel()
{
    m_impl->cancel();
}

}  // namespace markshot::recording
