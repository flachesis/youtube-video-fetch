project (youtube_video_fetch)
cmake_minimum_required(VERSION 2.8)

include_directories(/usr/local/include)
link_directories(/usr/local/lib)

add_executable(yvideof youtube.cpp)

target_link_libraries(yvideof curl)
