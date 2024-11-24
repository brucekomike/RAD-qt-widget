cmake_minimum_required(VERSION 3.16)

project(ros-copilot VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR}  REQUIRED COMPONENTS Widgets LinguistTools)
# debug flag
if(DEBUG)
set(CMAKE_BUILD_TYPE Debug)
endif()
set(EXECUTABLE deploy-cli)
message(${CMAKE_SYSTEM_PROCESSOR})

set(CUSTOM_PACKAGES)

if(MSYS)
  message("-------------")
  message("MSYS detected")
  message("-------------")
  message("this environment do not support static linking")
  message("thus not useable for system without MSYS unix emulation")
  if(CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64")
    set(EXECUTABLE deploy-cli_msys_x64)
  endif()
endif()

if(MINGW)
  message("--------------")
  message("MinGW detected")
  message("--------------")
  if(CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64")
    set(EXECUTABLE deploy-cli_mingw_x64)
  endif()
  set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
  if(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
    set(EXECUTABLE deploy-cli_mingw_arm64)
  endif()
  
endif()


if(LINUX AND NOT APPLE)
  message("--------------")
  message("Linux detected")
  message("--------------")
  if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    set(EXECUTABLE deploy-cli_linux_x64)
  endif()
  set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ ")
  set(CUSTOM_PACKAGES
    WaylandClient
  )
endif()



if(APPLE)
  message("---------------")
  message("Darwin detected")
  message("---------------")
  set(CMAKE_C_STANDARD 11)
  set(CMAKE_CXX_STANDARD 17)
  link_directories("/opt/homebrew/lib")
  include_directories(/opt/homebrew/include)
  if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    message(STATUS "Detected x86_64 architecture")
    # x86_64 specific settings here
    set(EXECUTABLE deploy-cli_darwin_x64)
  elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
    message(STATUS "Detected ARM64 architecture")
    set(EXECUTABLE deploy-cli_darwin_arm64)
    # Add ARM64 specific settings here
    if(CROSS_X64)
      message(STATUS "Cross-compiling for MacOS x64 platform")
      set(EXECUTABLE deploy-cli_darwin_x64)
      set(CMAKE_OSX_ARCHITECTURES "x86_64")
      set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15")
      #set(CMAKE_C_FLAGS "--target=x86_64-apple-darwin10.15 -mcpu=pentium2 ${CMAKE_CXX_FLAGS}")
      #set(CMAKE_CXX_FLAGS " --target=x86_64-apple-darwin10.15 -mcpu=pentium2 ${CMAKE_CXX_FLAGS}")
    else()
    set(CMAKE_OSX_DEPLOYMENT_TARGET "13.6")
    endif()
  endif()

endif()


# use this to fetch external library
include(FetchContent)

message("finding yaml-cpp ...")
find_package(yaml-cpp QUIET)
if(yaml-cpp_FOUND)
  message("yaml-cpp found")
else()
  message("yaml-cpp not found")
  message("fetching from github...")
  FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG 0.8.0 # Can be a tag (yaml-cpp-x.x.x), a commit hash, or a branch name (master)
  )
  FetchContent_MakeAvailable(yaml-cpp)
endif()

find_package(CLI11 QUIET)
if(CLI11_FOUND)
  message("CLI11 found")
else()
  message("CLI11 not found")
  message("fetching from github...")
  FetchContent_Declare(
    CLI11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG v2.4.2  # Use the latest stable version
  )
  FetchContent_MakeAvailable(CLI11)
  
endif()
# Fetch CLI11


set(TS_FILES ros-copilot_zh_CN.ts)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        configparser.h
        configparser.cpp
        mainwindow.ui
        ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    find_package(Qt6 REQUIRED COMPONENTS ${CUSTOM_PACKAGES})
    qt_add_executable(ros-copilot
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET ros-copilot APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(ros-copilot SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(ros-copilot
            ${PROJECT_SOURCES}
            configparser.h
            configparser.cpp
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(ros-copilot PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(ros-copilot PRIVATE CLI11::CLI11 yaml-cpp)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.ros-copilot)
endif()
set_target_properties(ros-copilot PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS ros-copilot
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)


if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ros-copilot)
endif()
