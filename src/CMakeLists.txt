generate_arduino_library(memory
    SRCS ${PROJECT_SOURCE_DIR}/ext/memory/memory.cpp
    HDRS ${PROJECT_SOURCE_DIR}/ext/memory/memory.h)

generate_arduino_library(infrared
    SRCS ${PROJECT_SOURCE_DIR}/ext/infrared/IRremote.cpp
    HDRS ${PROJECT_SOURCE_DIR}/ext/infrared/IRremote.h)

generate_arduino_library(radio
    SRCS ${PROJECT_SOURCE_DIR}/ext/radio/HCRadio.cpp
    HDRS ${PROJECT_SOURCE_DIR}/ext/radio/HCRadio.h)

generate_arduino_firmware(homecontrol
    SRCS main.cpp server.cpp tcp_utils.cpp http_request.cpp
    LIBS memory infrared radio)
