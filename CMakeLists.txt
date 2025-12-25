cmake_minimum_required(VERSION 3.20)

project(QSoftRenderer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")    

find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_subdirectory(src)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/test TEST_LIST)
file(GLOB HEADER_LIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "test/*.h")

add_executable(${PROJECT_NAME} ${TEST_LIST} ${HEADER_LIST})

target_include_directories(${PROJECT_NAME} PRIVATE src)
target_include_directories(${PROJECT_NAME} PRIVATE include)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE render
)
