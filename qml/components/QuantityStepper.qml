import QtQuick
import WaveOrdering

// Compact +/- stepper used on product cards, the product detail dialog and
// basket rows. The owner decides what "increase"/"decrease" means (e.g. add
// to cart vs. just bump a local quantity).
Row {
    id: root
    spacing: 0
    height: 36

    property int quantity: 0
    property int minimumValue: 0

    signal increase()
    signal decrease()

    Rectangle {
        width: 36
        height: 36
        radius: Theme.radiusSmall
        color: Theme.background
        border.color: Theme.border
        border.width: 1

        Text {
            anchors.centerIn: parent
            text: "−"
            font.pixelSize: 18
            color: Theme.textPrimary
        }

        MouseArea {
            anchors.fill: parent
            enabled: root.quantity > root.minimumValue
            onClicked: root.decrease()
        }
    }

    Item {
        width: 44
        height: 36

        Text {
            anchors.centerIn: parent
            text: root.quantity
            font.pixelSize: Theme.fontSizeBody
            font.bold: true
            color: Theme.textPrimary
        }
    }

    Rectangle {
        width: 36
        height: 36
        radius: Theme.radiusSmall
        color: Theme.primary

        Text {
            anchors.centerIn: parent
            text: "+"
            font.pixelSize: 18
            color: Theme.textOnPrimary
        }

        MouseArea {
            anchors.fill: parent
            onClicked: root.increase()
        }
    }
}
