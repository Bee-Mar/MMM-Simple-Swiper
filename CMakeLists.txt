cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 14)
project(MMM-Simple-Swiper)

if(NOT EXISTS "/usr/local/include/wiringPi.h")
  message("wiringPi dependency not met. Collecting and building wiringPi...\n")
  execute_process(COMMAND git clone git://git.drogon.net/wiringPi && cd wiringPi && ./build)
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/node_modules")
  message("-- node_modules directory not found project root: Running 'npm install'")
  execute_process(COMMAND npm install)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  # defines the macro used for debug mode within the executable
  add_definitions(-DMMM_SIMPLE_SWIPER_DEBUG)
  message("-- Compiling with MMM_SIMPLE_SWIPER_DEBUG flag set")
  # can be turned on from terminal using the following command:
  # cmake -DCMAKE_BUILD_TYPE=Debug ..
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# all related to linking wiringPi
find_library(WIRINGPI_LIBRARIES NAMES wiringPi)
find_path(WIRINGPI_INCLUDE_DIRS NAMES wiringPi.h)
include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(wiringPi # library name
    DEFAULT_MSG
    WIRINGPI_LIBRARIES
    WIRINGPI_INCLUDE_DIRS
  )

# all related to linking the boost library
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost REQUIRED
    COMPONENTS
    system  # boost::system
    thread  # boost::thread
  )

find_package(Threads REQUIRED) # linking pthread

include_directories(${Boost_INCLUDE_DIRS})

add_executable(mmm_simple_swiper # executable
    # source code files
    ${PROJECT_SOURCE_DIR}/CXX/main.cpp
    ${PROJECT_SOURCE_DIR}/CXX/swiper.cpp
    ${PROJECT_SOURCE_DIR}/CXX/Sensor.cpp
  )

target_link_libraries(mmm_simple_swiper # executable
    # libraries
    ${Boost_LIBRARIES}
    ${WIRINGPI_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    crypt
  )

