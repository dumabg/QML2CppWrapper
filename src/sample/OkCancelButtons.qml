import QtQuick 2.9
Row {
    id: r
    signal okClicked
    signal cancelClicked
    property alias okEnabled: ok.enabled
    property bool disableButtonsOnClicked: false
    spacing: 20
    anchors.horizontalCenter: parent.horizontalCenter
    OkButton {
        id: ok
        onClicked: {
            buttonClicked()
            okClicked()
        }
    }
    CancelButton {
        id: cancel
        onClicked: {
            buttonClicked()
            cancelClicked()
        }
    }
    function buttonClicked() {
        if (disableButtonsOnClicked) {
            ok.enabled = false
            cancel.enabled = false
        }
    }
}
