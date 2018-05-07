import QtQuick 2.9
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.3
Page {
    signal importStudents
    signal deleteCheckeds
    signal exportCsv
    signal addUser
    signal itemChecked(int index, bool checked)
    id: r
    anchors.fill: parent
    focus: true
    Keys.forwardTo: [students]
    Action {
        id: aExportCsv
        enabled: students.count !== 0
        icon {
            source: "csv.png"
        }
        onTriggered: r.exportCsv()
    }
    Action {
        id: aAdd
        icon {
            source: "add_user.png"
        }
        onTriggered: r.addUser()
    }
    Action {
        id: aImportStudents
        objectName: "actionImportStudents"
        icon {
            source: "touchscreen_smartphone.png"
        }
        onTriggered: {
            enabled = false
            r.importStudents()
        }
    }
    header: ToolBar {
        Row {
            height: parent.height
            TableButtons {
                table: students
            }
            ToolButton {
                action: aExportCsv
            }
            ToolButton {
                action: aAdd
            }
            ToolButton {
                action: aImportStudents
            }
        }
        Row {
            height: parent.height
            anchors.right: parent.right
            anchors.rightMargin: 10
            Text2 {
                text: qsTr("<b>%1</b> alumnos").arg(students.count)
                verticalAlignment: Text.AlignVCenter
                height: parent.height
            }
        }
    }
    Table {
        id: students
        anchors.fill: parent
        anchors.topMargin: 10
        anchors.leftMargin: 10
        rowHeight: 50
        model: c_students
        columns: [
            TableColumn {
                width: 300
                header: TableColumnHeader {
                    text: qsTr("Alumno")
                }
                content: TableColumnContent {
                    anchors.leftMargin: 60
                    text: model.name
                    Image {
                        id: img
                        x: -60
                        y: 2
                        width: 45
                        height: 45
                        source: model.photoSrc
                        layer.enabled: true
                        layer.effect: OpacityMask {
                            maskSource: Item {
                                width: img.width
                                height: img.height
                                Rectangle {
                                    anchors.centerIn: parent
                                    width: img.width
                                    height: img.height
                                    radius: 3
                                }
                            }
                        }
                    }
                }
            },
            TableColumn {
                width: 150
                header: TableColumnHeader {
                    text: qsTr("Matrícula")
                }
                content: TableColumnContent {
                    text: model.studentId
                }
            },
            TableColumn {
                width: 150
                header: TableColumnHeader {
                    text: qsTr("Correo")
                }
                content: TableColumnContent {
                    text: model.email
                }
            }
        ]
        Text2 {
            visible: students.count === 0
            font.pointSize: 14
            text: qsTr("Sin alumnos")
            anchors.centerIn: parent
        }
    }
    Component.onCompleted: students.itemChecked.connect(itemChecked)
}
