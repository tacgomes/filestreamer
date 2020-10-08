cmake_minimum_required(VERSION 3.10)

project(filestreamer)

set(CMAKE_CXX_STANDARD 14)

# Google Test

configure_file(cmake/CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                 ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                 EXCLUDE_FROM_ALL)

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

# Unit tests

enable_testing()

add_executable(testratelimit tests/testratelimiter.cpp)
target_link_libraries(testratelimit filestreamer gtest_main)
add_test(NAME rate_limit_test COMMAND testratelimit)

add_executable(teststreaming
               tests/teststreaming.cpp
               src/fileuploader/fileuploader.cpp
               src/filereceiver/filereceiver.cpp)
target_include_directories(teststreaming PUBLIC src/fileuploader src/filereceiver)
target_link_libraries(teststreaming filestreamer gtest_main)
add_test(NAME streaming_test COMMAND teststreaming)
