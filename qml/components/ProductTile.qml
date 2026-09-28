import QtQuick
import QtQuick.Controls
import WaveOrdering

// Placeholder product image: the category's initials on a tinted tile. Each
// category gets a consistent colour from Theme.categoryColors.
Rectangle {
    id: tile

    property int categoryId: -1
    property string categoryName

    readonly property color tone: {
        const colors = Theme.categoryColors
        const i = Math.abs(categoryId) % colors.length
        return colors[i]
    }

    function initials(name) {
        const words = name.split(/[^A-Za-z]+/).filter(w => w.length > 0)
        if (words.length === 0)
            return "?"
        if (words.length === 1)
            return words[0].substring(0, 2).toUpperCase()
        return (words[0][0] + words[1][0]).toUpperCase()
    }

    implicitWidth: 48
    implicitHeight: 48
    radius: Theme.radiusSmall + 2
    color: Qt.rgba(tone.r, tone.g, tone.b, 0.12)

    Label {
        anchors.centerIn: parent
        text: tile.initials(tile.categoryName)
        color: tile.tone
        font.pixelSize: 16
        font.bold: true
        font.letterSpacing: 0.5
    }
}
