cmake_minimum_required(VERSION 3.5.0)
project(lab_7 VERSION 0.1.0 LANGUAGES C CXX)

find_package(Boost REQUIRED COMPONENTS program_options)
add_executable(lab_7 main.cpp)
target_link_libraries(lab_7 PRIVATE Boost::program_options)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/directory.txt data.txt
    ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
