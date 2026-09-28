import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import WaveOrdering

// Centred placeholder for empty lists: icon, title, message and an optional
// call-to-action button.
ColumnLayout {
    id: root

    property url iconSource
    property string title
    property string message
    property string actionText

    signal actionTriggered()

    spacing: Theme.spacingSmall
    width: Math.min(parent ? parent.width - Theme.spacingLarge * 2 : 300, 320)

    Rectangle {
        Layout.alignment: Qt.AlignHCenter
        Layout.bottomMargin: Theme.spacingSmall
        width: 72
        height: 72
        radius: 36
        color: Qt.rgba(Theme.primaryLight.r, Theme.primaryLight.g, Theme.primaryLight.b, 0.12)

        Image {
            anchors.centerIn: parent
            source: root.iconSource
            sourceSize: Qt.size(64, 64)
            width: 32
            height: 32
            opacity: 0.8
        }
    }

    Label {
        Layout.fillWidth: true
        text: root.title
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
        font.pixelSize: Theme.fontSizeSubtitle
        font.bold: true
        color: Theme.textPrimary
    }

    Label {
        Layout.fillWidth: true
        visible: text.length > 0
        text: root.message
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
        font.pixelSize: Theme.fontSizeBody
        color: Theme.textSecondary
    }

    Button {
        Layout.alignment: Qt.AlignHCenter
        Layout.topMargin: Theme.spacingSmall
        visible: root.actionText.length > 0
        text: root.actionText
        highlighted: true
        onClicked: root.actionTriggered()
    }
}
