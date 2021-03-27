set(CMAKE_DEBUG_POSTFIX d)

cmake_minimum_required(VERSION 3.15)
project( lib.ffmpeg )

if(NOT USE_CUDA)
    message(STATUS "USE_CUDA have not been set. Set USE_CUDA = OFF")
    option(USE_CUDA "Use CUDA" OFF)
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__STDC_CONSTANT_MACROS")
if(MSVC)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
endif(MSVC)

file(GLOB HEADERS 
    *.hpp
    NVDECHelpers/NvCodec/NvDecoder/*.h
    NVDECHelpers/Utils/NvDecoder/*.h
)
file(GLOB SOURCES 
    *.cpp
    NVDECHelpers/NvCodec/NvDecoder/*.cpp
    NVDECHelpers/Utils/NvDecoder/*.cpp
    NVDECHelpers/Utils/NvDecoder/*.cu
)

if(USE_CUDA)
  file(GLOB CUDA_SOURCES "*.cu")
endif()

# FFmpeg
set(FFMPEG_INCLUDE_DIRS "${FFMPEG_DIR}/include")
set(FFMPEG_LIBDIR "${FFMPEG_DIR}/include")

find_library(AVCODEC_LIB    avcodec    HINTS ${FFMPEG_LIBDIR})
find_library(AVFORMAT_LIB   avformat   HINTS ${FFMPEG_LIBDIR})
find_library(AVUTIL_LIB     avutil     HINTS ${FFMPEG_LIBDIR})
find_library(AVDEVICE_LIB   avdevice   HINTS ${FFMPEG_LIBDIR})
find_library(SWSCALE_LIB    swscale    HINTS ${FFMPEG_LIBDIR})
find_library(SWRESAMPLE_LIB swresample HINTS ${FFMPEG_LIBDIR})
set(FFMPEG_LIBRARIES ${AVCODEC_LIB} ${AVFORMAT_LIB} ${AVUTIL_LIB} ${AVDEVICE_LIB} ${SWSCALE_LIB} ${SWRESAMPLE_LIB})

# CUDA
include(CheckLanguage)
check_language(CUDA)
if (USE_CUDA)
        set(CUDA_ARCHITECTURES "Auto" CACHE STRING "\"Auto\" detects local machine GPU compute arch at runtime, \"Common\" and \"All\" cover common and entire subsets of architectures, \"Names\" is a list of architectures to enable by name, \"Numbers\" is a list of compute capabilities (version number) to enable")
        set_property(CACHE CUDA_ARCHITECTURES PROPERTY STRINGS "Auto" "Common" "All" "Kepler Maxwell Kepler+Tegra Maxwell+Tegra Pascal" "5.0 7.5")
        #set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-gencode arch=compute_20,code=sm_20)
        enable_language( CUDA )
        if(CMAKE_CUDA_COMPILER_VERSION VERSION_LESS "10.0")
            message(STATUS "Unsupported CUDA version, please upgrade to CUDA 10+. Disabling CUDA support")
            set(USE_CUDA "OFF" CACHE BOOL "Enable CUDA support" FORCE)
        else()  
            find_package( CUDA )
            cuda_select_nvcc_arch_flags(CUDA_ARCH_FLAGS ${CUDA_ARCHITECTURES})
            message(STATUS "USE_CUDA: ${USE_CUDA}")
            message(STATUS "Found CUDA ${CUDA_VERSION_STRING} at ${CUDA_TOOLKIT_ROOT_DIR}")
            message(STATUS "Building with CUDA flags: " "${CUDA_ARCH_FLAGS}")
        endif()
        set(CMAKE_CUDA_RUNTIME_LIBRARY "Static")
endif()

add_library( ${PROJECT_NAME} ${HEADERS} ${SOURCES} ${CUDA_SOURCES} )

if (USE_CUDA AND CUDA_FOUND)
    set_target_properties( ${PROJECT_NAME} PROPERTIES CUDA_SEPARABLE_COMPILATION ON )
endif()

target_include_directories( ${PROJECT_NAME} PUBLIC ${FFMPEG_INCLUDE_DIRS} )
if (USE_CUDA AND CUDA_FOUND)
    target_include_directories( ${PROJECT_NAME} PRIVATE ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES} )
endif()

target_link_libraries( ${PROJECT_NAME} PUBLIC ${FFMPEG_LIBRARIES} )
if (USE_CUDA AND CUDA_FOUND)
    target_link_libraries( ${PROJECT_NAME} PRIVATE cuda nvcuvid )
endif()

if (USE_CUDA AND CUDA_FOUND)
    target_compile_definitions( ${PROJECT_NAME} PRIVATE -DWITH_CUDA )
endif()

message(STATUS "Found FFmpeg libraries: ${FFMPEG_LIBRARIES}")