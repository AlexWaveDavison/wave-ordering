import QtQuick
import QtQuick.Controls
import WaveOrdering

Rectangle {
    id: chip

    property string categoryName
    property bool selected: false

    signal clicked()

    implicitWidth: label.implicitWidth + Theme.spacingMedium * 2
    implicitHeight: 34
    radius: height / 2
    color: selected ? Theme.primary : (tap.pressed ? Theme.border : Theme.surface)
    border.color: selected ? Theme.primary : Theme.border
    border.width: 1

    Behavior on color { ColorAnimation { duration: 120 } }

    Label {
        id: label
        anchors.centerIn: parent
        text: chip.categoryName
        color: chip.selected ? Theme.textOnPrimary : Theme.textPrimary
        font.pixelSize: Theme.fontSizeBody - 1
        font.bold: chip.selected
    }

    MouseArea {
        id: tap
        anchors.fill: parent
        onClicked: chip.clicked()
    }

    Accessible.role: Accessible.RadioButton
    Accessible.name: categoryName
    Accessible.checked: selected
}
