cmake_minimum_required(VERSION 3.15)
project(Cena_like_Demo)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Core Widgets Gui Network REQUIRED)

add_executable(Cena_like_Demo server.ui client.ui server.cpp client.cpp main.cpp common.cpp)

target_link_libraries(Cena_like_Demo Qt5::Core Qt5::Widgets Qt5::Gui Qt5::Network)

qt5_use_modules(Cena_like_Demo Core Widgets Gui Network)