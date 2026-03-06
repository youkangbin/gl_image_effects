# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(gl_image_effects LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/quick/scenegraph/gl_image_effects")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick)

qt_standard_project_setup()

qt_add_executable(gl_image_effects
    main.cpp
    squircle.cpp squircle.h
)

set_target_properties(gl_image_effects PROPERTIES

    # Prevent name clash with build subdirectory on case-insensitive file systems
    OUTPUT_NAME openglunderqmlapp
)

set_target_properties(gl_image_effects PROPERTIES
    OUTPUT_NAME openglunderqmlapp
    # Android 需要这个属性
    QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android"
)

target_link_libraries(gl_image_effects PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
)

if(WIN32)
    set_target_properties(gl_image_effects PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

qt_add_qml_module(gl_image_effects
    URI OpenGLUnderQML
    QML_FILES main.qml
    RESOURCE_PREFIX /scenegraph/gl_image_effects
    NO_RESOURCE_TARGET_PATH
)

install(TARGETS gl_image_effects
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
