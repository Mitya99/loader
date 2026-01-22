cmake_minimum_required(VERSION 3.16)

project(q_Loader LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-D_GNU_SOURCE)

add_executable(q_Loader main.cpp
    Loader.cpp Loader.h requests.cpp requests.h)

target_include_directories(q_Loader PRIVATE
    /opt/homebrew/include               # boost
    ${CMAKE_CURRENT_SOURCE_DIR}/extern  # nlohmann
)

include(GNUInstallDirs)
install(TARGETS q_Loader
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
