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
            // Layout.preferredHeight: form.height * 0.75
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
            Button  {
                text: qsTr("清除全选")
                font.pixelSize: 12
                onClicked: {
                    for (var i = 0; i < filterModel.count; i++)
                        filterModel.setProperty(i, "enabled", false)
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
                orientation: ListView.Horizontal
                snapMode: ListView.SnapToItem
                spacing: 2

                delegate: Rectangle {
                    // ========== 核心：设置方形尺寸 ==========
                    width: 80   // 方块宽度（可根据需求调整）
                    height: 80  // 方块高度，与宽度一致形成正方形
                    radius: 8   // 圆角增大，更像方块按钮
                    color: model.enabled
                           ? Qt.rgba(Theme.mainColor.r, Theme.mainColor.g, Theme.mainColor.b, 0.3)  // 选中态
                           : (itemMouse.containsMouse ? "#3a3a5e" : "#2a2a3e")  // hover/默认态

                    Behavior on color { ColorAnimation { duration: 120 } }

                    // ========== 内部布局：垂直居中排列复选框+文字 ==========
                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 6  // 内边距，避免内容贴边
                        spacing: 6          // 复选框和文字的间距

                        // 复选框（居中显示）
                        Rectangle {
                            Layout.alignment: Qt.AlignHCenter  // 水平居中
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

                        // 滤镜名称（居中显示，自动换行）
                        Text {
                            Layout.alignment: Qt.AlignHCenter  // 水平居中
                            Layout.fillWidth: true             // 占满宽度，方便换行
                            text: model.name
                            color: model.enabled ? "white" : "#aaaacc"
                            font.pixelSize: 12                 // 缩小字体适配方块
                            horizontalAlignment: Text.AlignHCenter  // 文字居中
                            verticalAlignment: Text.AlignTop
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere  // 文字过长时换行
                            maximumLineCount: 2  // 最多显示2行，避免超出方块
                        }
                    }

                    // 交互区域（铺满整个方块）
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
