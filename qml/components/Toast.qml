import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import WaveOrdering

// Short-lived message shown above the tab bar (errors, confirmations).
Rectangle {
    id: toast

    function show(message) {
        label.text = message
        hideTimer.restart()
        state = "shown"
    }

    width: Math.min(label.implicitWidth + Theme.spacingLarge * 2,
                    (parent ? parent.width : 400) - Theme.spacingMedium * 2)
    height: label.implicitHeight + Theme.spacingMedium * 1.5
    radius: Theme.radiusMedium
    color: Theme.primaryDark
    opacity: 0
    visible: opacity > 0
    z: 100

    states: State {
        name: "shown"
        PropertyChanges { target: toast; opacity: 0.95 }
    }

    transitions: Transition {
        NumberAnimation { property: "opacity"; duration: 180; easing.type: Easing.OutCubic }
    }

    Label {
        id: label
        anchors.centerIn: parent
        width: Math.min(implicitWidth, toast.width - Theme.spacingLarge * 2)
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignHCenter
        color: Theme.textOnPrimary
        font.pixelSize: Theme.fontSizeBody - 1
    }

    Timer {
        id: hideTimer
        interval: 3200
        onTriggered: toast.state = ""
    }

    MouseArea {
        anchors.fill: parent
        onClicked: toast.state = ""
    }
}
