# FFmpegWrapper
**lib.ffmpeg** library for demuxing, parsing and decoding video streams.

## Build
Build with pure cmake
```
mkdir build && cd build
cmake -DFFMPEG_DIR=F:\FFmpeg\ffmpeg-4.1 -DUSE_CUDA=OFF ..
```
or use .vscode folder for building in VS code

## Run
You can run demo for decoding video stream
```
FFmpegWrapper.exe <path_to_video>
```
