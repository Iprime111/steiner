cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(steiner LANGUAGES CXX)
include(cmake/CPM.cmake)

CPMAddPackage(
    NAME fmt
    GITHUB_REPOSITORY fmtlib/fmt
    GIT_TAG 12.1.0
)

CPMAddPackage(
    NAME CLI11
    GITHUB_REPOSITORY CLIUtils/CLI11
    VERSION 2.6.2
)

CPMAddPackage(
    NAME nlohmann_json
    GITHUB_REPOSITORY nlohmann/json
    VERSION 3.12.0
)

add_executable(${PROJECT_NAME})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)

target_link_libraries( ${PROJECT_NAME} PRIVATE 
    nlohmann_json::nlohmann_json 
    fmt::fmt
    CLI11
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)
endif()

set(STEINER_SRC
    types.cpp
    algo.cpp
    main.cpp
)

list(TRANSFORM STEINER_SRC PREPEND src/)
target_sources(${PROJECT_NAME} PRIVATE ${STEINER_SRC})

target_include_directories(${PROJECT_NAME} PRIVATE include)