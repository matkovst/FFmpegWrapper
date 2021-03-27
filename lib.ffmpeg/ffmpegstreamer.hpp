#pragma once

#include <iostream>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavutil/file.h>
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavutil/log.h>
}

#ifdef WITH_CUDA
#include <cuda.h>
#include <cuda_runtime.h>
#include <cuviddec.h>
#include <nvcuvid.h>
#endif

#include "utils.hpp"

class Frame;


/**
 * Wrapper class for using ffmpeg utilities.
 */
class FFmpegStreamer
{
public:
    FFmpegStreamer() = default;

    FFmpegStreamer(const char* filename);

    ~FFmpegStreamer();

    int openStream(const char* filename);

    /* Allocate Frame and set its fields to video stream values */
    Frame allocFrame() const;

    int read(Frame& frame);

    FFmpegStreamer& operator >> (Frame& frame);

    operator bool() const noexcept;

private:
    AVFormatContext* m_formatCtx { NULL };
    AVIOContext* m_avioCtx { NULL };
    AVCodecContext* m_videoDecCtx { NULL };
    struct SwsContext* m_swsCtx { NULL };

    uint8_t* m_buffer { NULL };
    uint8_t* m_avioCtxBuffer { NULL };
    size_t m_bufferSize; 
    size_t m_avioCtxBufferSize { 4096 };
    char* m_inputFilename { NULL };
    int m_ret { 0 };
    struct buffer_data m_bufferData { 0 };

    int m_videoStreamIdx { -1 };
    AVStream* m_videoStream { NULL };
    int m_width { 0 }; 
    int m_height { 0 };
    enum AVPixelFormat m_pixFormat;
    int m_outWidth { 0 };
    int m_outHeight { 0 };
    AVPixelFormat m_outPixFormat { AV_PIX_FMT_BGR24 };

    AVPacket m_tmpAVPacket { 0 };
    AVFrame* m_tmpAVFrame;

    int m_readOk { 0 }; // 0 - success
    int m_sendOk { 0 }; // 0 - success
    int m_receiveOk { 0 }; // 0 - success
};
