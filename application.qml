// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick.Controls
import Qt.labs.platform
import Theme

ApplicationWindow {
    id: window
    visible: true
    width: 420
    height: 680
    minimumWidth: 360
    minimumHeight: 500
    title: qsTr("图片滤镜工具")

    MainForm {
        id: form
        anchors.fill: parent

        // 载入图片按钮 → 打开文件对话框
        button.onClicked: fileDialog.open()
    }

    FileDialog {
        id: fileDialog
        title: qsTr("选择图片")
        nameFilters: ["图片文件 (*.png *.jpg *.jpeg *.bmp)"]
        onAccepted: {
            // TODO: 将 fileDialog.currentFile 传给 OpenGLItem 加载纹理
            console.log("选中文件：", fileDialog.currentFile)
        }
    }
}
