cmake_minimum_required(VERSION 3.16)

project(async_fps_work_pool LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(work_pool_demo
    main.cpp
)

target_compile_features(work_pool_demo PRIVATE cxx_std_20)
target_include_directories(work_pool_demo PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/work_pool
)
