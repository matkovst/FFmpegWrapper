#include "ffmpegstreamer.hpp"
#include "frame.hpp"

extern "C"
{
#include <libswscale/swscale.h>
}

FFmpegStreamer::FFmpegStreamer(const char* filename)
{
    openStream(filename);
}

FFmpegStreamer::~FFmpegStreamer()
{
    if (m_formatCtx != NULL)
    {
    avformat_close_input(&m_formatCtx);
    }
    /* note: the internal buffer could have changed, and be != avio_ctx_buffer */
    if (m_avioCtx) 
    {
        av_freep(&m_avioCtx->buffer);
        av_freep(&m_avioCtx);
        avio_context_free(&m_avioCtx);
    }
    av_file_unmap(m_buffer, m_bufferSize);
    if (m_ret < 0) 
    {
        char errbuf[256];
        av_strerror(m_ret, errbuf, sizeof(errbuf));
        std::cerr << "Error occurred while destructing FFmpegStreamer: " << errbuf;
    }

    av_packet_unref(&m_tmpAVPacket);
    if (m_tmpAVFrame)
    {
        av_freep(m_tmpAVFrame->data);
        av_frame_free(&m_tmpAVFrame);
    }
    
    avcodec_free_context(&m_videoDecCtx);

    sws_freeContext(m_swsCtx);
}

int FFmpegStreamer::openStream(const char* filename)
{
    m_formatCtx = avformat_alloc_context();
    m_inputFilename = const_cast<char*>(filename);
    
    /* register codecs and formats and other lavf/lavc components*/
    av_register_all();

    /* slurp file content into buffer */
    m_ret = av_file_map(m_inputFilename, &m_buffer, &m_bufferSize, 0, NULL);
    if (m_ret < 0)
    {
        std::cerr << "Could not slurp file content into buffer" << std::endl;
        return m_ret;
    }
    
    /* fill opaque structure used by the AVIOContext read callback */
    m_bufferData.ptr  = m_buffer;
    m_bufferData.size = m_bufferSize;
    if (!m_formatCtx) 
    {
        m_ret = AVERROR(ENOMEM);
        return m_ret;
    }
    m_avioCtxBuffer = (uint8_t*)av_malloc(m_avioCtxBufferSize);
    if (!m_avioCtxBuffer) 
    {
        m_ret = AVERROR(ENOMEM);
        return m_ret;
    }
    m_avioCtx = avio_alloc_context(m_avioCtxBuffer, m_avioCtxBufferSize, 0, &m_bufferData, &read_packet, NULL, NULL);
    if (!m_avioCtx) 
    {
        m_ret = AVERROR(ENOMEM);
        return m_ret;
    }
    m_formatCtx->pb = m_avioCtx;
    m_ret = avformat_open_input(&m_formatCtx, NULL, NULL, NULL);
    if (m_ret < 0) 
    {
        std::cerr << "Could not open input\n";
        return m_ret;
    }
    m_ret = avformat_find_stream_info(m_formatCtx, NULL);
    if (m_ret < 0) 
    {
        std::cerr << "Could not find stream information\n";
        return m_ret;
    }


    if (open_codec_context(&m_videoStreamIdx, &m_videoDecCtx, m_formatCtx, AVMEDIA_TYPE_VIDEO) >= 0) 
    {
        m_videoStream = m_formatCtx->streams[m_videoStreamIdx];

        /* allocate image where the decoded image will be put */
        m_width = m_videoDecCtx->width;
        m_height = m_videoDecCtx->height;
        m_pixFormat = m_videoDecCtx->pix_fmt;
        m_outWidth = m_width;
        m_outHeight = m_height;
        m_outPixFormat = AV_PIX_FMT_BGR24;
    }

    m_tmpAVFrame = av_frame_alloc();

    /* create scaling context */
    m_swsCtx = sws_getContext(m_width, m_height, m_pixFormat,
                            m_outWidth, m_outHeight, m_outPixFormat,
                            SWS_AREA, NULL, NULL, NULL);
    if (!m_swsCtx) 
    {
        std::cerr << "Impossible to create scale context for the conversion" << std::endl;
        m_ret = AVERROR(EINVAL);
        return m_ret;
    }

    // av_dump_format(m_formatCtx, 0, filename, 0);

    return 0;
}

Frame FFmpegStreamer::allocFrame() const
{
    return Frame(m_outWidth, m_outHeight, m_outPixFormat);
}

int FFmpegStreamer::read(Frame& frame)
{
    /* Get the next raw frame of stream */
    av_packet_unref(&m_tmpAVPacket);
    av_frame_unref(m_tmpAVFrame);
    m_readOk = av_read_frame(m_formatCtx, &m_tmpAVPacket);
    if ( m_readOk < 0 ) 
    {
        return m_readOk;
    }

    /* Decode raw frame */
    do
    {
        m_sendOk = avcodec_send_packet(m_videoDecCtx, &m_tmpAVPacket);
    } 
    while ( ( m_receiveOk = avcodec_receive_frame(m_videoDecCtx, m_tmpAVFrame) ) != 0 );
    
    if ( m_sendOk == 0 )
    {
        /* Convert frame to destination format (color space and scale) */
        AVFrame* pFrame = frame.frame();
        sws_scale(m_swsCtx, (const uint8_t * const*)m_tmpAVFrame->data,
                    m_tmpAVFrame->linesize, 0, m_height, pFrame->data, pFrame->linesize);
    }

    return 0;
}

FFmpegStreamer& FFmpegStreamer::operator >> (Frame& frame)
{
    read(frame);
    return *this;
}

FFmpegStreamer::operator bool() const noexcept
{
    return m_readOk == 0 && m_receiveOk == 0;
}