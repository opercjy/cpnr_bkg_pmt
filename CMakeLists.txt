cmake_minimum_required(VERSION 3.16...3.27)
project(RI_decay)

find_package(Geant4 REQUIRED ui_all vis_all)
include(${Geant4_USE_FILE})

# Geant4 11.x 필수 C++17 표준 강제
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SOURCES "src/*.cc")
file(GLOB HEADERS "include/*.hh")
include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(RI_Sim main.cc ${SOURCES})
target_link_libraries(RI_Sim ${Geant4_LIBRARIES})

# ==================================================================
# 매크로 폴더를 빌드 디렉토리로 자동 복사
# ==================================================================
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/macros
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
