import QtQuick
import QtQuick.Controls
import WaveOrdering

// Small rounded pill showing an order's status.
Rectangle {
    id: chip

    property bool sent: false
    property string text

    readonly property color tone: sent ? Theme.success : Theme.warning

    implicitWidth: label.implicitWidth + 16
    implicitHeight: 22
    radius: height / 2
    color: Qt.rgba(tone.r, tone.g, tone.b, 0.12)

    Label {
        id: label
        anchors.centerIn: parent
        text: chip.text
        color: chip.tone
        font.pixelSize: Theme.fontSizeCaption - 1
        font.bold: true
    }
}
