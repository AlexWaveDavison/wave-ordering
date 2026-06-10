import QtQuick
import WaveOrdering

Rectangle {
    id: chip

    property string categoryName: ""
    property int categoryId: -1
    property bool selected: false

    signal clicked()

    width: label.implicitWidth + Theme.spacingMedium * 2
    height: 36
    radius: height / 2
    color: selected ? Theme.primary : Theme.surface
    border.color: selected ? Theme.primary : Theme.border
    border.width: 1

    Text {
        id: label
        anchors.centerIn: parent
        text: chip.categoryName
        color: chip.selected ? Theme.textOnPrimary : Theme.textPrimary
        font.pixelSize: Theme.fontSizeBody
        font.bold: chip.selected
    }

    MouseArea {
        anchors.fill: parent
        onClicked: chip.clicked()
    }
}
