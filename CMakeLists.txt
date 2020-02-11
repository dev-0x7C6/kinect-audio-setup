cmake_minimum_required(VERSION 3.2)
project(kinect-audio-setup)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
find_package(libusb-1.0 REQUIRED)
include_directories(${LIBUSB_1_INCLUDE_DIRS})

option(UDEV "install udev rules" ON)

if (UDEV)
    install(FILES udev/55-kinect_audio.rules DESTINATION /lib/udev/rules.d)
endif()

set(kinect_upload_fw_src
  src/kinect_upload_fw.c
)

add_executable (kinect_upload_fw ${kinect_upload_fw_src})
target_link_libraries (kinect_upload_fw ${LIBUSB_1_LIBRARIES})

install(TARGETS kinect_upload_fw DESTINATION bin)
