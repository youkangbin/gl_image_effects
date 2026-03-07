// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Theme
import MyComponents 1.0

Item {
    id: form
    width: 400
    height: 640

    // 对外暴露的 alias（flatstyle.qml 里还在用 button，保留空壳即可）
    property alias button: loadButton

    // 滤镜数据模型
    ListModel {
        id: filterModel
        ListElement { name: "灰度 (Grayscale)";    enabled: false }
        ListElement { name: "反色 (Invert)";        enabled: false }
        ListElement { name: "模糊 (Blur)";          enabled: false }
        ListElement { name: "锐化 (Sharpen)";       enabled: false }
        ListElement { name: "边缘检测 (Edge)";      enabled: false }
        ListElement { name: "暖色调 (Warm)";        enabled: false }
        ListElement { name: "冷色调 (Cool)";        enabled: false }
        ListElement { name: "复古 (Sepia)";         enabled: false }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        // ── ① OpenGL 渲染区域 ──────────────────────────
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: form.height * 0.55
            color: "#111111"
            radius: 6
            clip: true

            OpenGLItem {
                id: glView
                anchors.fill: parent
            }

            // 左上角提示文字
            Text {
                text: "预览窗口"
                color: "#88ffffff"
                font.pixelSize: 11
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.margins: 6
            }
        }

        // ── ② 操作按钮行 ───────────────────────────────
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Button {
                id: loadButton
                text: qsTr("载入图片")
                Layout.fillWidth: true
            }

            Button {
                id: loadLUT
                text: qsTr("载入LUT")
                Layout.fillWidth: true
            }

            Button {
                id: loadMASK
                text: qsTr("载入MASK")
                Layout.fillWidth: true
            }

            Button {
                id: applyButton
                text: qsTr("应用滤镜")
                Layout.fillWidth: true
                // 高亮已有选中滤镜时才可点击
                enabled: {
                    for (var i = 0; i < filterModel.count; i++)
                        if (filterModel.get(i).enabled) return true
                    return false
                }
            }

            Button {
                id: exportButton
                text: qsTr("导出图片")
                Layout.fillWidth: true
            }
        }

        // ── ③ 滤镜列表标题 ─────────────────────────────
        RowLayout {
            Layout.fillWidth: true

            Text {
                text: qsTr("滤镜列表（可多选）")
                font.pixelSize: 13
                font.bold: true
                color: Theme.mainColor
            }

            Item { Layout.fillWidth: true }   // spacer

            // 一键清除所有选中
            Text {
                text: qsTr("清除全选")
                font.pixelSize: 12
                color: "#888888"
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        for (var i = 0; i < filterModel.count; i++)
                            filterModel.setProperty(i, "enabled", false)
                    }
                }
            }
        }

        // ── ④ 滤镜复选列表 ─────────────────────────────
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#1a1a2e"
            radius: 6
            border.color: "#333355"
            border.width: 1
            clip: true

            ListView {
                id: filterList
                anchors.fill: parent
                anchors.margins: 4
                model: filterModel
                spacing: 2

                delegate: Rectangle {
                    width: filterList.width
                    height: 40
                    radius: 4
                    color: model.enabled ? Qt.rgba(
                               Theme.mainColor.r,
                               Theme.mainColor.g,
                               Theme.mainColor.b, 0.25)
                                         : (itemMouse.containsMouse ? "#2a2a3e" : "transparent")

                    Behavior on color { ColorAnimation { duration: 120 } }

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        anchors.rightMargin: 10
                        spacing: 10

                        // 复选框
                        Rectangle {
                            width: 18; height: 18
                            radius: 3
                            color: model.enabled ? Theme.mainColor : "transparent"
                            border.color: model.enabled ? Theme.mainColor : "#666688"
                            border.width: 1.5

                            Text {
                                anchors.centerIn: parent
                                text: "✓"
                                color: "white"
                                font.pixelSize: 12
                                font.bold: true
                                visible: model.enabled
                            }
                        }

                        // 滤镜名称
                        Text {
                            text: model.name
                            color: model.enabled ? "white" : "#aaaacc"
                            font.pixelSize: 13
                            Layout.fillWidth: true
                        }
                    }

                    MouseArea {
                        id: itemMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: filterModel.setProperty(
                                       index, "enabled", !model.enabled)
                    }
                }
            }
        }
    }
}
