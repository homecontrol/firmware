set(CMAKE_TOOLCHAIN_FILE ext/cmake/cmake/ArduinoToolchain.cmake)

cmake_minimum_required(VERSION 2.8)

project(HomeControl C CXX)

print_board_list()
print_programmer_list()

set(ARDUINO_DEFAULT_BOARD ethernet)
set(ARDUINO_DEFAULT_PORT /dev/ttyACM0)

include_directories(ext)
add_subdirectory(src)
