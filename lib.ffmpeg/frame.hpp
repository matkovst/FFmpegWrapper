#pragma once

extern "C"
{
#include <libavutil/avutil.h>
}

/**
 * Class wrapper for AVFrame.
 */
class Frame
{
public:
    Frame()
    {
        allocFrame();
    }

    Frame(int width, int height, AVPixelFormat pixelFormat)
    {
        allocFrame();

        av_image_alloc(m_AVFrame->data, m_AVFrame->linesize, width, height, pixelFormat, 32);
        m_AVFrame->width = width;
        m_AVFrame->height = height;
        m_AVFrame->format = pixelFormat;
    }

    ~Frame()
    {
        av_frame_free(&m_AVFrame);
    }

    AVFrame* frame() const noexcept { return m_AVFrame; }

    operator bool() const noexcept
    {
        return m_AVFrame != nullptr && m_AVFrame->pict_type != AV_PICTURE_TYPE_NONE;
    }

private:
    AVFrame* m_AVFrame;

    /* Allocate Frame and set its fields to default values */
    void allocFrame()
    {
        m_AVFrame = av_frame_alloc();
        if (!m_AVFrame)
        {
            throw "Frame: Could not allocate memory for AVFrame";
        }
    }
};

using SharedFrame = std::shared_ptr<Frame>;