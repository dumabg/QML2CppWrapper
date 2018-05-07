import QtQuick 2.9
import QtQuick.Controls 2.3
Row {
    signal deleteCheckeds
    property var table
    Action {
        id: aSelect
        enabled: table.count !== 0
        icon {
            source: "img/checklist.png"
        }
        onTriggered: table.checkable = !table.checkable
    }
    Action {
        id: aDelete
        icon {
            source: "img/delete.png"
        }
        enabled: (table.checkable) && (table.count > 0)
        onTriggered: r.deleteCheckeds()
    }
    ToolButton {
        action: aSelect
        highlighted: table.checkable
    }
    Row {
        visible: table.checkable
        ToolButton {
            action: aDelete
        }
        Item {
            width: 10
            Line {
                anchors.right: parent.right
                height: parent.height
                width: 1
            }
            height: parent.height
        }
        Item {
            width: 10
            height: 1
        }
    }
}
