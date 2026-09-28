import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import WaveOrdering

// One catalogue row. Role values are injected by the ListView via the
// required properties below.
Rectangle {
    id: card

    required property int productId
    required property int categoryId
    required property string categoryName
    required property string name
    required property string sku
    required property string unit
    required property string priceText
    required property bool isPoa
    required property int quantityInCart

    signal openDetails()

    radius: Theme.radiusMedium
    color: tap.pressed ? Qt.rgba(Theme.border.r, Theme.border.g, Theme.border.b, 0.4) : Theme.surface
    border.color: quantityInCart > 0 ? Theme.primaryLight : Theme.border
    border.width: 1
    implicitHeight: Math.max(84, contentRow.implicitHeight + 24)

    // Declared before the content so the stepper's own buttons sit on top.
    MouseArea {
        id: tap
        anchors.fill: parent
        onClicked: card.openDetails()
    }

    RowLayout {
        id: contentRow
        anchors.fill: parent
        anchors.margins: 12
        spacing: 12

        ProductTile {
            Layout.alignment: Qt.AlignTop
            categoryId: card.categoryId
            categoryName: card.categoryName
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2

            Label {
                Layout.fillWidth: true
                text: card.name
                font.pixelSize: Theme.fontSizeBody
                font.bold: true
                color: Theme.textPrimary
                wrapMode: Text.WordWrap
                maximumLineCount: 2
                elide: Text.ElideRight
            }

            Label {
                Layout.fillWidth: true
                text: card.sku.length > 0 ? qsTr("%1 · %2").arg(card.sku).arg(card.unit) : card.unit
                font.pixelSize: Theme.fontSizeCaption
                color: Theme.textSecondary
                elide: Text.ElideRight
            }

            Label {
                Layout.topMargin: 2
                text: card.isPoa ? qsTr("Price on application") : card.priceText
                font.pixelSize: Theme.fontSizeBody
                font.bold: !card.isPoa
                font.italic: card.isPoa
                color: card.isPoa ? Theme.textSecondary : Theme.primary
            }
        }

        QuantityStepper {
            Layout.alignment: Qt.AlignVCenter
            showAddWhenZero: true
            quantity: card.quantityInCart
            onIncrease: CartModel.increment(card.productId)
            onDecrease: CartModel.decrement(card.productId)
        }
    }
}
