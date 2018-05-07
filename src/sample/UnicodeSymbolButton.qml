import QtQuick 2.9
import QtQuick.Controls 2.2
RoundButton {
    property string symbol
    property color color
    id: control
    radius: 2
    width: item.implicitWidth
    height: item.implicitHeight
    contentItem: Item {
        width: control.width
        height: control.height
        Row {
            id: item
            anchors.centerIn: parent
            padding: 10
            spacing: 5
            Text2 {
                text: control.symbol
                font.family: "Segoe UI Symbol"
                font.pointSize: control.font.pointSize + 2
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
            }
            Text2 {
                text: control.text
                font: control.font
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }
    background: Rectangle {
        id: back
        width: control.width
        height: control.height
        radius: control.radius
        color: control.enabled ?
                   (control.down ? Qt.darker(control.color) : control.color) :
                   "lightgray"
    }
}
