import QtQuick 2.0

Rectangle {
    id: line
    signal xDragged(real newX)
    property real minimumX: 0
    property real maximumX: 0
//    property bool dragged: false
    property alias dragActive: maLine.dragActive
    color: "lightgray"
    width: 1
    height: parent.height
    MouseArea {
        property bool dragActive: drag.active
        id: maLine
        anchors.fill: parent
        cursorShape: Qt.SplitHCursor
        drag.maximumX: line.maximumX
        drag.minimumX: line.minimumX
        drag.target: line
        drag.axis: Drag.XAxis
        drag.threshold: 0
        onMouseXChanged: xDragged(parent.x)
//        onDragActiveChanged: {
//            if (!dragActive) {
//                line.dragged = true
//            }
//        }
    }
}
