// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Theme
import MyComponents 1.0

Item {
    id: form
    anchors.fill: parent

    property alias button: loadButton

    // ── 滤镜 key 映射（与 C++ FilterFlag 和 filter.frag 中常量一致）──
    readonly property var filterKeys: [
        "grayscale", "invert", "blur", "sharpen", "edge", "warm", "cool", "sepia"
    ]

    // 收集当前激活的 key 列表，推送给 OpenGLItem
    function buildActiveFilters() {
        var result = []
        for (var i = 0; i < filterModel.count; i++)
            if (filterModel.get(i).enabled)
                result.push(filterKeys[i])
        return result
    }

    ListModel {
        id: filterModel
        ListElement { name: "灰度\nGrayscale";  enabled: false }
        ListElement { name: "反色\nInvert";     enabled: false }
        ListElement { name: "模糊\nBlur";       enabled: false }
        ListElement { name: "锐化\nSharpen";    enabled: false }
        ListElement { name: "边缘\nEdge";       enabled: false }
        ListElement { name: "暖色\nWarm";       enabled: false }
        ListElement { name: "冷色\nCool";       enabled: false }
        ListElement { name: "复古\nSepia";      enabled: false }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 6

        // ── ① OpenGL 渲染区域（主体，占满剩余空间）──────────
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#111111"
            radius: 6
            clip: true

            OpenGLItem {
                id: glView
                anchors.fill: parent
                imagePath: ":/images/lenna.png"   // ← 默认图片
            }

            Text {
                text: "预览窗口"
                color: "#66ffffff"
                font.pixelSize: 10
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.margins: 5
            }
        }

        // ── ② 操作按钮行（紧凑，固定高度）──────────────────
        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 28
            spacing: 5

            Repeater {
                model: [
                    { id_ref: "loadImageButton",  label: qsTr("载入图片") },
                    { id_ref: "loadLUT",          label: qsTr("载入LUT")  },
                    { id_ref: "loadMASK",         label: qsTr("载入MASK") },
                    { id_ref: "applyButton",      label: qsTr("应用滤镜") },
                    { id_ref: "exportButton",     label: qsTr("导出图片") },
                ]

                Button {
                    objectName: modelData.id_ref
                    text: modelData.label
                    Layout.fillWidth: true
                    Layout.preferredHeight: 28
                    font.pixelSize: 11

                    enabled: objectName === "applyButton"
                             ? (function() {
                                   for (var i = 0; i < filterModel.count; i++)
                                       if (filterModel.get(i).enabled) return true
                                   return false
                               })()
                             : true

                    onClicked: {
                        if (objectName === "loadImageButton") {
                            loadButton.clicked()
                        }
                        // applyButton：点击后才真正推送滤镜到 OpenGLItem
                        if (objectName === "applyButton") {
                            glView.activeFilters = form.buildActiveFilters()
                        }
                    }
                }
            }
        }

        // alias 绑定用的隐藏按钮（供 flatstyle.qml 的 button.onClicked 使用）
        Button { id: loadButton; visible: false; text: "" }

        // ── ③ 滤镜区域（标题 + 横向列表）────────────────────
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 3

            // 标题行
            RowLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: 20

                Text {
                    text: qsTr("滤镜（可多选）")
                    font.pixelSize: 11
                    font.bold: true
                    color: Theme.mainColor
                    verticalAlignment: Text.AlignVCenter
                }

                Item { Layout.fillWidth: true }

                Button {
                    text: qsTr("清除")
                    font.pixelSize: 10
                    Layout.preferredHeight: 20
                    Layout.preferredWidth: 44
                    onClicked: {
                        for (var i = 0; i < filterModel.count; i++)
                            filterModel.setProperty(i, "enabled", false)
                        // ← 清除时同步重置 OpenGLItem
                        glView.activeFilters = []
                    }
                }
            }

            // 横向滤镜列表
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 68
                color: "#1a1a2e"
                radius: 5
                border.color: "#333355"
                border.width: 1
                clip: true

                ListView {
                    id: filterList
                    anchors.fill: parent
                    anchors.margins: 3
                    model: filterModel
                    orientation: ListView.Horizontal
                    snapMode: ListView.SnapToItem
                    spacing: 3

                    delegate: Rectangle {
                        width: 62
                        height: filterList.height
                        radius: 6
                        color: model.enabled
                               ? Qt.rgba(Theme.mainColor.r, Theme.mainColor.g, Theme.mainColor.b, 0.3)
                               : (itemMouse.containsMouse ? "#3a3a5e" : "#2a2a3e")

                        Behavior on color { ColorAnimation { duration: 100 } }

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 4
                            spacing: 3

                            Rectangle {
                                Layout.alignment: Qt.AlignHCenter
                                width: 14; height: 14
                                radius: 2
                                color: model.enabled ? Theme.mainColor : "transparent"
                                border.color: model.enabled ? Theme.mainColor : "#666688"
                                border.width: 1.2

                                Text {
                                    anchors.centerIn: parent
                                    text: "✓"
                                    color: "white"
                                    font.pixelSize: 9
                                    font.bold: true
                                    visible: model.enabled
                                }
                            }

                            Text {
                                Layout.alignment: Qt.AlignHCenter
                                Layout.fillWidth: true
                                text: model.name
                                color: model.enabled ? "white" : "#aaaacc"
                                font.pixelSize: 9
                                horizontalAlignment: Text.AlignHCenter
                                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                maximumLineCount: 2
                                lineHeight: 1.1
                            }
                        }

                        MouseArea {
                            id: itemMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                filterModel.setProperty(index, "enabled", !model.enabled)
                                // ← 勾选/取消时不立即生效，等用户点"应用滤镜"
                                // 如需实时预览，改为：glView.activeFilters = form.buildActiveFilters()
                            }
                        }
                    }
                }
            }
        }
    }
}
