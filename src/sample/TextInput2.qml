import QtQuick 2.9
import QtQuick.Controls 2.2
FocusScope {
    property alias caption: t.text
    property alias text: ti.text
    property alias source: image.source
    property alias font: ti.font
    property alias inputMethodHints: ti.inputMethodHints
    property alias inputMask: ti.inputMask
    signal buttonClicked
    width: c.implicitWidth
    height: c.implicitHeight
    clip: true
    focus: true
    Column {
        id: c
        width: parent.width
        spacing: 2
        Text2 {
            id: t
            renderType: Text.NativeRendering
            font.bold: ti.activeFocus
        }
        Rectangle {
            width: parent.width
            height: ti.implicitHeight + 10
            border.color: ti.activeFocus ? "blue" : "black"
            color: "transparent"
            TextInput {
                id: ti
                anchors.fill: parent
                anchors.leftMargin: 4
                anchors.rightMargin: b.visible ? b.width + 4 : 4
                verticalAlignment: Qt.AlignVCenter
                renderType: Text.NativeRendering
                selectByMouse: true
                focus: true
            }
            Button {
                id: b
                y: 2
                flat: true
                focusPolicy: Qt.NoFocus
                visible: image.status !== Image.Null
                height: parent.height - 4
                width: image.implicitWidth + 6
                anchors.right: parent.right
                anchors.rightMargin: 1
                Image {
                    id: image
                    anchors.centerIn: parent
                }
                onClicked: buttonClicked()
            }
        }
    }
}
