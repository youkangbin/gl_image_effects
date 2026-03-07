# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(gl_image_effects LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/quickcontrols/gl_image_effects")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick QuickControls2 Widgets OpenGL)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

qt_add_executable(gl_image_effects WIN32 MACOSX_BUNDLE
    main.cpp
    openglitem.cpp
    "res.qrc"
)

qt_add_qml_module(gl_image_effects
    URI gl_image_effects
    NO_RESOURCE_TARGET_PATH
    QML_FILES
        "application.qml"
        "MainForm.qml"
        "imports/Flat/Button.qml"
        "imports/Flat/CheckBox.qml"
        "imports/Flat/Switch.qml"
        "imports/Theme/Theme.qml"
    RESOURCES
        "imports/Flat/qmldir"
        "imports/Theme/qmldir"
        "qtquickcontrols2.conf"
)

target_link_libraries(gl_image_effects PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Widgets
    Qt6::OpenGL
)

install(TARGETS gl_image_effects
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
