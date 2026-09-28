import QtQuick
import QtQuick.Controls
import WaveOrdering

// Quantity control used on product cards, the product sheet and basket rows.
// The owner decides what increase/decrease mean (update the basket vs. bump
// a local value). With `showAddWhenZero`, a zero quantity shows a single
// "Add" button instead of "− 0 +".
Item {
    id: root

    property int quantity: 0
    property int minimumValue: 0
    property bool showAddWhenZero: false

    signal increase()
    signal decrease()

    readonly property bool addMode: showAddWhenZero && quantity === 0

    implicitWidth: addMode ? addButton.implicitWidth : stepperRow.implicitWidth
    implicitHeight: 36

    Rectangle {
        id: addButton
        visible: root.addMode
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        implicitWidth: addLabel.implicitWidth + 32
        height: 36
        radius: 18
        color: addTap.pressed ? Theme.primaryDark : Theme.primary

        Label {
            id: addLabel
            anchors.centerIn: parent
            text: qsTr("Add")
            color: Theme.textOnPrimary
            font.pixelSize: Theme.fontSizeBody - 1
            font.bold: true
        }

        MouseArea {
            id: addTap
            anchors.fill: parent
            onClicked: root.increase()
        }

        Accessible.role: Accessible.Button
        Accessible.name: qsTr("Add to basket")
    }

    Row {
        id: stepperRow
        visible: !root.addMode
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        spacing: 0

        Rectangle {
            readonly property bool enabledButton: root.quantity > root.minimumValue
            width: 36
            height: 36
            radius: 18
            color: minusTap.pressed ? Theme.border : Theme.background
            border.color: Theme.border
            border.width: 1
            opacity: enabledButton ? 1 : 0.4

            Label {
                anchors.centerIn: parent
                text: "−"
                font.pixelSize: 20
                color: Theme.textPrimary
            }

            MouseArea {
                id: minusTap
                anchors.fill: parent
                enabled: parent.enabledButton
                onClicked: root.decrease()
            }

            Accessible.role: Accessible.Button
            Accessible.name: qsTr("Decrease quantity")
        }

        Label {
            width: 40
            height: 36
            text: root.quantity
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: Theme.fontSizeBody
            font.bold: true
            color: Theme.textPrimary
        }

        Rectangle {
            width: 36
            height: 36
            radius: 18
            color: plusTap.pressed ? Theme.primaryDark : Theme.primary

            Label {
                anchors.centerIn: parent
                text: "+"
                font.pixelSize: 20
                color: Theme.textOnPrimary
            }

            MouseArea {
                id: plusTap
                anchors.fill: parent
                onClicked: root.increase()
            }

            Accessible.role: Accessible.Button
            Accessible.name: qsTr("Increase quantity")
        }
    }
}
