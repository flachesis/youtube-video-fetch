project (youtube_video_fetch)
cmake_minimum_required(VERSION 2.8)

add_subdirectory( BehaviorDB )

if(MINGW)
include_directories(${PROJECT_SOURCE_DIR} E:/code_lib/curl/include BehaviorDB)
link_directories(${PROJECT_SOURCE_DIR} E:/code_lib/curl/lib)

add_executable(yvideof youtube.cpp)
add_executable(yvideoBDB youtubeWithBDB.cpp YoutubeCrawler.cpp)
target_link_libraries(yvideof curldll wsock32)
target_link_libraries(yvideoBDB curldll wsock32 bdb)
else()
include_directories(${PROJECT_SOURCE_DIR} /usr/local/include BehaviorDB)
link_directories(${PROJECT_SOURCE_DIR} /usr/local/lib)

add_executable(yvideof youtube.cpp)
add_executable(yvideoBDB youtubeWithBDB.cpp YoutubeCrawler.cpp)
target_link_libraries(yvideof curl)
target_link_libraries(yvideoBDB curl bdb)
endif()
