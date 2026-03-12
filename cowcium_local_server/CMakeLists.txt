cmake_minimum_required(VERSION 3.16)
project(cowcium_local_server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GPIOD REQUIRED libgpiod)

add_executable(cowcium_local_server
    src/main.cpp
    src/gpio_power.cpp
    src/color_sensor.cpp
    src/http_server.cpp
)

target_include_directories(cowcium_local_server
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${GPIOD_INCLUDE_DIRS}
)

target_link_directories(cowcium_local_server PRIVATE ${GPIOD_LIBRARY_DIRS})
target_link_libraries(cowcium_local_server PRIVATE ${GPIOD_LIBRARIES})
target_compile_options(cowcium_local_server PRIVATE ${GPIOD_CFLAGS_OTHER})
