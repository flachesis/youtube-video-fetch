project (youtube_video_fetch)
cmake_minimum_required(VERSION 2.8)

if(MINGW)
include_directories(E:/code_lib/curl/include)
link_directories(E:/code_lib/curl/lib)

add_executable(yvideof youtube.cpp)
target_link_libraries(yvideof curldll wsock32)
else()
include_directories(/usr/local/include)
link_directories(/usr/local/lib)

add_executable(yvideof youtube.cpp)
target_link_libraries(yvideof curl)
endif()
