import QtQuick 2.9
import QtQuick.Controls 2.3
ListView {
    id: r
    signal itemChecked(int index, bool checked)
    property color headerColor: "#3498db"
    property real headerHeight: 25
    property real rowHeight: 25
    property color rowAlternateColor: "#FBFBFB"
    property color currentColor: "#CCE8FF"
    property color hoveredColor: "#E5F3FF"
    property color rowColor: "white"
    property list<TableColumn> columns
    property bool checkable: false
    onCheckableChanged: calcContentWidth()
    readonly property int checkableWidth: 26
    onColumnsChanged: calcContentWidth()
    focus: true
    flickableDirection: Flickable.AutoFlickIfNeeded
    headerPositioning: ListView.OverlayHeader
    clip: true
    header: Rectangle {
        id: h
        z: 3
        height: r.headerHeight
        color: r.headerColor
        width: r.contentWidth
        Row {
            id: row
            spacing: 0
            Item {
                visible: r.checkable
                width:  r.checkableWidth
                height: 1
            }
            Repeater {
                id: repeater
                model: columns.length
                Row {
                    spacing: 0
                    Item {
                        width: columns[index].width
                        height: r.headerHeight
                        data: columns[index].header
                    }
                    VerticalLineDraggable {
                        enabled: columns.length > 1
                        maximumX: r.contentWidth
                        minimumX: 20
                        onXDragged: {
                            var dif = newX - columns[index].width
                            columns[index].width = columns[index].width + dif
                            var item = repeater.itemAt(index)
                            item.children[0].width = columns[index].width
                            var offset = 0
                            for (var i = 0; i < index; i++) {
                                offset += columns[i].width + 5
                            }
                            item.children[1].maximumX = r.width - offset - 2
                            calcContentWidth()
                        }
                    }
                }
            }
        }
    }
    delegate: CheckDelegate {
        id: rowData
        property int rowIndex: index
        property bool isCurrentItem: ListView.isCurrentItem
        property var itemModel: model
        width: row2.width
        height: r.rowHeight
        padding: 0
        contentItem : Row {
            id: row2
            spacing: 0
            Item {
                visible: r.checkable
                width:  r.checkableWidth
                height: 1
            }
            Repeater {
                model: columns.length
                Item {
                    id: item
                    property int indexContent: index
                    width: columns[index].width
                    height: r.rowHeight
                    clip: true
                    Loader {
                        id: loader
                        anchors.fill: parent
                        property var model: rowData.itemModel
                        property int index: rowData.rowIndex
                        sourceComponent: columns[item.indexContent].content
                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: checkItem()
                        }
                    }
                }
            }
        }
        background: Rectangle {
            width: rowData.implicitWidth + columns.length
            height: rowData.implicitHeight
            color: ((rowData.isCurrentItem) && (!r.checkable)) || ((r.checkable) && (rowData.checked)) ? r.currentColor :
                                           (rowData.hovered ? r.hoveredColor :
                                                                  ((rowData.rowIndex % 2) == 0 ? r.rowColor : r.rowAlternateColor))
        }
        indicator: Item {
            width: r.checkableWidth
            height: r.rowHeight
            Rectangle {
                anchors.centerIn: parent
                visible: r.checkable
                width: 14
                height: 14
                radius: 3
                color: "transparent"
                border.color: r.headerColor
                Rectangle {
                    width: 8
                    height: 8
                    x: 3
                    y: 3
                    radius: 2
                    color: r.headerColor
                    visible: rowData.checked
                }
            }
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onClicked: checkItem()
            }
        }
        function checkItem() {
            if (r.checkable) {
                rowData.checked = !rowData.checked
                r.itemChecked(rowData.rowIndex, rowData.checked)
            }
            r.currentIndex = rowData.rowIndex
        }
    }
    ScrollBar.horizontal: ScrollBar {
        policy: r.width < r.contentWidth ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
    }
    ScrollBar.vertical: ScrollBar {
        policy: r.height < r.contentHeight ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
    }
    MouseArea {
        z: -1
        anchors.fill: parent
        propagateComposedEvents: true
        onWheel: {
            if (wheel.angleDelta.y > 0) {
                if (r.currentIndex > 0) {
                    r.currentIndex -= 1
                }
            }
            else {
                if (r.currentIndex < r.count - 1) {
                    r.currentIndex += 1
                }
            }
        }
    }
    Keys.onDownPressed: {
        if (r.currentIndex < r.count - 1)
            r.currentIndex++
     }
     Keys.onUpPressed: {
         if (r.currentIndex > 0)
             r.currentIndex --
     }
    function calcContentWidth() {
        var totalWidth = r.checkable ? r.checkableWidth : 0
        for (var i = 0; i < columns.length; i++) {
            totalWidth += columns[i].width + 1
        }
        r.contentWidth = totalWidth
    }
}
