import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Theme
import MyComponents 1.0
import QtQuick.Controls 2.5

Item {
    id: form
    anchors.fill: parent

    property alias button: loadButton
    property alias glView: glView

    readonly property var filterKeys: [
        "grayscale", "invert", "blur", "sharpen", "edge", "warm", "cool", "sepia", "pixel"
    ]

    function buildActiveFilters() {
        var result = []
        for (var i = 0; i < filterModel.count; i++)
            if (filterModel.get(i).enabled)
                result.push(filterModel.get(i).key)
        return result
    }

    function isFilterEnabled(key) {
        for (var i = 0; i < filterModel.count; i++)
            if (filterModel.get(i).key === key && filterModel.get(i).enabled)
                return true
        return false
    }

    ListModel {
        id: filterModel
        ListElement { name: "模糊\nBlur";    key:"blur";      enabled: false }
        ListElement { name: "锐化\nSharpen";  key:"sharpen";   enabled: false }
        ListElement { name: "LUT";           key:"lut";       enabled: false }
        ListElement { name: "MASK";          key:"mask";      enabled: false }
        ListElement { name: "灰度\nGrayscale"; key:"grayscale"; enabled: false }
        ListElement { name: "反色\nInvert";    key:"invert";    enabled: false }
        ListElement { name: "边缘\nEdge";      key:"edge";      enabled: false }
        ListElement { name: "暖色\nWarm";      key:"warm";      enabled: false }
        ListElement { name: "冷色\nCool";      key:"cool";      enabled: false }
        ListElement { name: "复古\nSepia";     key:"sepia";     enabled: false }
        ListElement { name: "像素\nPixel";     key:"pixel";     enabled: false }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 6

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#111111"
            radius: 6
            clip: true

            OpenGLItem {
                id: glView
                anchors.fill: parent
                imagePath: ":/images/lenna.png"
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

        Button { id: loadButton; visible: false; text: "" }

        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 28
            spacing: 5

            Repeater {
                model: [
                    { id_ref: "loadImageButton",  label: qsTr("载入图片") },
                    { id_ref: "loadLUT",          label: qsTr("载入LUT")  },
                    { id_ref: "loadMASK",         label: qsTr("载入MASK") },
                    { id_ref: "exportButton",     label: qsTr("导出图片") },
                    { id_ref: "clear",            label: qsTr("清除") },
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
                        if (objectName === "applyButton") {
                            glView.activeFilters = form.buildActiveFilters()
                        }
                        if (objectName === "clear") {
                            for (var i = 0; i < filterModel.count; i++)
                                filterModel.setProperty(i, "enabled", false)
                            glView.activeFilters = []
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            visible: form.isFilterEnabled("sharpen")

            Text {
                text: qsTr("锐化强度")
            }

            Slider {
                id: slider
                Layout.fillWidth: true
                height: 10
                from: 0
                to: 100
                value: 0
                stepSize: 1
                onValueChanged: {
                    glView.sharpenIntensity = value / 100.0
                }
            }

            Text {
                id: valueText
                text: Math.round(slider.value)
                width: 30
            }
        }

        RowLayout {
            Layout.fillWidth: true
            visible: form.isFilterEnabled("blur")

            Text {
                text: qsTr("模糊强度")
            }

            Slider {
                id: blurSlider
                Layout.fillWidth: true
                height: 10
                from: 0
                to: 100
                value: 0
                stepSize: 1
                onValueChanged: {
                    glView.blurIntensity = value / 100.0
                }
            }

            Text {
                text: Math.round(blurSlider.value)
                width: 30
            }
        }

        RowLayout {
            Layout.fillWidth: true
            visible: form.isFilterEnabled("pixel")

            Text {
                text: qsTr("像素强度")
            }

            Slider {
                id: pixelSlider
                Layout.fillWidth: true
                height: 10
                from: 1
                to: 100
                value: 1
                stepSize: 1
                onValueChanged: {
                    glView.pixelIntensity = value / 100.0
                }
            }

            Text {
                text: Math.round(pixelSlider.value)
                width: 30
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 3

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
                                glView.activeFilters = form.buildActiveFilters()
                            }
                        }
                    }
                }
            }
        }
    }
}
