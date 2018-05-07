import QtQuick 2.9
import QtQuick.Window 2.3
Window {
    signal selectPhoto
    signal okClicked
    signal cancelClicked
    property alias name: name.text
    readonly property alias studentId: studentId.text
    readonly property alias email: email.text
    width: 300
    height: 320
    visible: true
    flags: Qt.Dialog | Qt.MSWindowsFixedSizeDialogHint | Qt.WindowTitleHint
    modality: Qt.ApplicationModal
    Column {
        id: content
        width: parent.width - 20
        x: 10
        y: 10
        spacing: 20
        TextInput2 {
            id: name
            width: parent.width
            font.capitalization: Font.Capitalize
            caption: qsTr("Name")
        }
        TextInput2 {
            id: studentId
            width: 100
            caption: qsTr("Id")
            inputMethodHints: Qt.ImhDigitsOnly
            inputMask: "99/9999999"
        }
        TextInput2 {
            id: email
            width: parent.width
            font.capitalization: Font.AllLowercase
            caption: qsTr("Email")
            inputMethodHints: Qt.ImhEmailCharactersOnly | Qt.ImhLowercaseOnly
        }
        Column {
            spacing: 3
            Text2 {
                text: qsTr("Photo")
                wrapMode: Text.WordWrap
            }
            Rectangle {
                width: 45
                height: width
                border.color: "black"
                border.width: 1
                color: "transparent"
                radius: 3
                smooth: true
                Image {
                    id: img
                    objectName: "photo"
                    width: 45
                    height: 45
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: selectPhoto()
                }
            }
        }
        OkCancelButtons {
            id: buttons
            okEnabled: (name.text.trim().length > 0) && (studentId.text.trim().length > 0)
                                     && ((email.text.indexOf('.', email.text.indexOf('@'))) > -1)
        }
    }
    Component.onCompleted: {
        buttons.onOkClicked.connect(okClicked)
        buttons.onCancelClicked.connect(cancelClicked)
    }
}
