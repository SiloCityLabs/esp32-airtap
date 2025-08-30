cmake_minimum_required(VERSION 3.16.0)

# Set the ESP-IDF components directory to include our custom components
set(EXTRA_COMPONENT_DIRS 
    ${CMAKE_CURRENT_SOURCE_DIR}/components/esp-zigbee-sdk/components
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(zigbee-4btn-rev2)
