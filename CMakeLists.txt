cmake_minimum_required(VERSION 3.10)

project(filestreamer)

set(CMAKE_CXX_STANDARD 14)

# File streamer

include_directories(src/lib)

add_compile_options(-Wall -Wextra -pedantic -Werror)

file(GLOB FILE_RECEIVER_SOURCES "src/lib/*.cpp")
add_library(filestreamer STATIC ${FILE_RECEIVER_SOURCES})

file(GLOB FILE_UPLOADER_SOURCES "src/fileuploader/*.cpp")
add_executable(fileuploader ${FILE_UPLOADER_SOURCES})
target_link_libraries(fileuploader filestreamer)

file(GLOB FILE_RECEIVER_SOURCES "src/filereceiver/*.cpp")
add_executable(filereceiver ${FILE_RECEIVER_SOURCES})
target_link_libraries(filereceiver filestreamer)
