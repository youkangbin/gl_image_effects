// Copyright (C) 2017 The Qt Company Ltd.
import QtQuick
import QtQuick.Controls
import Qt.labs.platform
import Theme
import MyComponents 1.0          // ← ✅ 新增这一行

ApplicationWindow {
    id: window
    visible: true
    minimumWidth: 360
    height: 480
    title: qsTr("Flat Style")

    MainForm {
        id: form
        anchors.fill: parent
        button.onClicked: colorDialog.open()
        sizeSwitch.onCheckedChanged: Theme.baseSize = (sizeSwitch.checked ? Theme.largeSize : Theme.smallSize)
        checkBoxBold.onCheckedChanged: Theme.font.bold = checkBoxBold.checked
        checkBoxUnderline.onCheckedChanged: Theme.font.underline = checkBoxUnderline.checked
        slider.onPositionChanged: Theme.font.pixelSize = slider.valueAt(slider.position)

        // ← ✅ 新增这一块，嵌入 OpenGL 渲染窗口
        OpenGLItem {
            width: 300
            height: 300
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 20
        }
    }

    ColorDialog {
        id: colorDialog
        onCurrentColorChanged: Theme.mainColor = currentColor
    }
}
