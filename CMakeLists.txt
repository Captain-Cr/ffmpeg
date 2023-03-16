cmake_minimum_required(VERSION 2.8)

project(playYUV)

#set(FFMPEG_LIB_PATH "/usr/local/ffmpeg/lib")
set(FFMPEG_INCLUDE "/usr/local/ffmpeg/include")
set(FFMPEG_LIBARY_DIR "/usr/local/ffmpeg/lib")

include_directories(${FFMPEG_INCLUDE})
link_directories(${FFMPEG_LIBARY_DIR})
link_libraries(SDL2)
set(FFMPEG_LIB libavcodec.so libavformat.so libswscale.so libavdevice.so libavutil.so)

add_executable(playYUV playYUV.cpp)
target_link_libraries(playYUV ${FFMPEG_LIB})

