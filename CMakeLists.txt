# test-scheduler project
#

cmake_minimum_required(VERSION 2.8.12)
project(mtl-trigger)

cmake_policy(SET CMP0087 NEW)
set(CMAKE_CXX_STANDARD 20)

include(status.cmake)
_begin_status()
# version
set(TS_VERSION "1.0.1")
_write_status("Test-Scheduler version: ${TS_VERSION}")
# git branch
_git_branch(branch)
_write_status("Branch: ${branch}")

if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    set(ARCH 64)
else( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    set(ARCH 32)
endif( CMAKE_SIZEOF_VOID_P EQUAL 8 )

if (NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
endif (NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
if (NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
endif (NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
if (NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
endif (NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)

# Qt5
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
find_package(Qt5 COMPONENTS Core Widgets SerialPort REQUIRED)
if(Qt5_FOUND)
  _write_status("Qt5 version: ${Qt5_VERSION}")
  set(LIBS ${LIBS} Qt::Core Qt::Widgets Qt::SerialPort)
  include_directories(${Qt5Widgets_INCLUDE_DIRS})
endif()

# MTL
find_library(MTL32 MTL32Calls)
set(LIBS ${LIBS} ${MTL32})

# libusb
if( ARCH EQUAL 64 )
  set(LIBUSB "${CMAKE_CURRENT_SOURCE_DIR}/libusb/lib/msvc_x64/libusb")
else( ARCH EQUAL 64 )
  set(LIBUSB "${CMAKE_CURRENT_SOURCE_DIR}/libusb/lib/msvc/libusb.lib")
endif( ARCH EQUAL 64 )
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/libusb/include")
set(LIBS ${LIBS} ${LIBUSB})

# install variables
set(INSTALL_PATH .)
set(BIN_COMPONENT BIN)

set(INSTALL_LIB_PATH ${INSTALL_PATH})
set(COMPONENT_NAME ${BIN_COMPONENT})

include_directories(
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_BINARY_DIR}"
  $ENV{INCLUDE}
)

set(SOURCES
    main.cpp
    mainwindow.cpp
    task.cpp
#    task_template.cpp
    MTLtask.cpp
    linefilebrowse.cpp
    com_trigger_task.cpp
    task_property.cpp
    keyvaluemodel.cpp
    digispark_trigger_task.cpp
    controller.cpp
)
set(MOCS
    mainwindow.h
)
set(HEADERS
    task.h
#    task_template.h
    MTLtask.h
    linefilebrowse.h
    com_trigger_task.h
    task_property.h
    togglebox.h
    keyvaluemodel.h
    digispark_trigger_task.h
    controller.h
)
set(UIS
    mainwindow.ui
)

# Build the app!
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(${PROJECT_NAME}
  WIN32
# source files that are actually built direclty
  ${SOURCES}
  ${GENERATED_SOURCES}

# items included so they show up in your IDE
  ${HEADERS}
  ${UIS}
  ${MOCS}
)

# Link the app!
target_link_libraries(${PROJECT_NAME} ${LIBS})

#
# install commands
#

# Create installation targets. Note that we put each kind of file
# into a different component via COMPONENT. These components will
# be used to create the installation components.
if (WIN32)
  install(CODE "execute_process(COMMAND windeployqt.exe -dir \"${PROJECT_BINARY_DIR}/_deployment\" \"$<TARGET_FILE:${PROJECT_NAME}>\")")
  install(DIRECTORY "${PROJECT_BINARY_DIR}/_deployment/"
          DESTINATION ${INSTALL_PATH})
endif (WIN32)

install(TARGETS ${PROJECT_NAME}
        RUNTIME
        DESTINATION ${INSTALL_PATH})

if (WIN32)
  set(CPACK_GENERATOR ZIP WIX)
endif (WIN32)

set(CPACK_PACKAGE_VERSION "${TS_VERSION}")

# CPack boilerplate for this project
set(CPACK_PACKAGE_NAME "mtl-trigger")
set(CPACK_PACKAGE_VENDOR "machine vision lab")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "MTL Trigger")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "MTL Trigger")
set(CPACK_PACKAGE_EXECUTABLES "${PROJECT_NAME};MTL Trigger")
set(CPACK_CREATE_DESKTOP_LINKS "${PROJECT_NAME};MTL Trigger")

# Set the displayed names for each of the components to install.
# These will be displayed in the list of components inside the installer.
set(CPACK_COMPONENT_${BIN_COMPONENT}_DISPLAY_NAME "MTL Trigger")
if (WIN32)
  set(CPACK_COMPONENT_QTLIBS_DISPLAY_NAME "Qt Libraries")
endif (WIN32)

_end_status()
# Include CPack to introduce the appropriate targets
include(CPack)