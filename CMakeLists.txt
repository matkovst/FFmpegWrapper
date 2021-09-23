cmake_minimum_required(VERSION 3.13)
project( FFmpegWrapper )

set(CMAKE_CXX_STANDARD 11)

# lib.ffmpeg
add_subdirectory(lib.ffmpeg)

# OpenCV
find_package( OpenCV REQUIRED COMPONENTS core imgproc videoio highgui )

add_executable( ${PROJECT_NAME} main.cpp )

target_include_directories( ${PROJECT_NAME} PUBLIC ${OpenCV_INCLUDE_DIRS} )

target_link_libraries( ${PROJECT_NAME} lib.ffmpeg ${OpenCV_LIBS} )

set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}" CACHE PATH "Install prefix" FORCE)
set(INSTALL_BIN_DIR "${CMAKE_SOURCE_DIR}" CACHE PATH "Path where exe and dll will be installed")
install(
    TARGETS ${PROJECT_NAME}
    CONFIGURATIONS Release
    RUNTIME DESTINATION bin
)