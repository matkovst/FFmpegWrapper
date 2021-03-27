#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "lib.ffmpeg/ffmpegstreamer.hpp"
// #include "lib.ffmpeg/ffmpegstreamerNVDEC.hpp"
#include "lib.ffmpeg/frame.hpp"

int main(int argc, char *argv[])
{
    std::cout << "Info: program started" << std::endl;

    /* Parse cmd args */
    if (argc != 2) 
    {
        std::cerr << "usage: %s input_file\n" 
            << "API example program to show how to read from a custom buffer " 
            << "accessed through AVIOContext.\n", argv[0];
        return 1;
    }

    /* FFmpeg magic. CPU-treatment */
    FFmpegStreamer ffStreamer;
    ffStreamer.openStream(argv[1]);

    Frame frame = ffStreamer.allocFrame();
    
    /* Looping */
    clock_t tStart = clock();
    std::int64_t fno = 0;
    while (ffStreamer >> frame)
    {
        cv::Mat img{ frame.frame()->height, frame.frame()->width, CV_8UC3, frame.frame()->data[0], (size_t)frame.frame()->linesize[0] };
        cv::imshow("Test", img);

        char c = (char)cv::waitKey(30);
        if( c == 27 || c == 'q' )
        {
            break;
        }
        ++fno;
    }
    double elapsed = static_cast<double>(clock() - tStart) / CLOCKS_PER_SEC;
    double avgTime = elapsed / fno;
    std::cout << "Info: average time to process frame " << avgTime << " sec" << std::endl;

    std::cout << "Info: program successfully finished" << std::endl;
    return 0;
}