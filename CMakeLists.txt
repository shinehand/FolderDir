cmake_minimum_required(VERSION 3.16)
project(FolderDir VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6 first, fall back to Qt5
find_package(Qt6 COMPONENTS
    Core
    Gui
    Widgets
    Concurrent
    QUIET
)

if(NOT Qt6_FOUND)
    find_package(Qt5 5.15 REQUIRED COMPONENTS
        Core
        Gui
        Widgets
        Concurrent
    )
    set(QT_VERSION_MAJOR 5)
else()
    set(QT_VERSION_MAJOR 6)
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    set(QT_LIBS Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Concurrent)
else()
    set(QT_LIBS Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Concurrent)
endif()

set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/FolderPane.cpp
    src/FileSystemBrowser.cpp
    src/FileSystemModel.cpp
    src/AddressBar.cpp
    src/FileOperations.cpp
    src/FileOperationDialog.cpp
    src/BookmarkManager.cpp
    src/PreviewPanel.cpp
    src/SettingsManager.cpp
    src/SettingsDialog.cpp
    src/SearchDialog.cpp
    src/DriveBar.cpp
)

set(HEADERS
    src/MainWindow.h
    src/FolderPane.h
    src/FileSystemBrowser.h
    src/FileSystemModel.h
    src/AddressBar.h
    src/FileOperations.h
    src/FileOperationDialog.h
    src/BookmarkManager.h
    src/PreviewPanel.h
    src/SettingsManager.h
    src/SettingsDialog.h
    src/SearchDialog.h
    src/DriveBar.h
)

if(WIN32)
    set(APP_ICON_RESOURCE "resources/app.rc")
endif()

add_executable(FolderDir WIN32 MACOSX_BUNDLE
    ${SOURCES}
    ${HEADERS}
    ${APP_ICON_RESOURCE}
    resources/resources.qrc
)

target_link_libraries(FolderDir PRIVATE ${QT_LIBS})

target_include_directories(FolderDir PRIVATE src)

# Platform-specific
if(WIN32)
    target_compile_definitions(FolderDir PRIVATE UNICODE _UNICODE)
endif()

# Install
include(GNUInstallDirs)
install(TARGETS FolderDir
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(FolderDir)
endif()
