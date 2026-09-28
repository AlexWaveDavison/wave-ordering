import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import WaveOrdering

Page {
    id: page

    signal browseRequested()
    signal reordered(int count)

    background: Rectangle { color: Theme.background }

    EmptyState {
        anchors.centerIn: parent
        visible: OrderModel.count === 0
        iconSource: "../icons/orders.png"
        title: qsTr("No orders yet")
        message: qsTr("Orders you place will be saved here so you can check, resend or reorder them.")
        actionText: qsTr("Browse catalogue")
        onActionTriggered: page.browseRequested()
    }

    ColumnLayout {
        anchors.fill: parent
        visible: OrderModel.count > 0
        spacing: 0

        Label {
            Layout.fillWidth: true
            Layout.margins: Theme.spacingMedium
            Layout.bottomMargin: Theme.spacingSmall
            text: OrderModel.pendingCount > 0
                  ? (OrderModel.pendingCount === 1
                     ? qsTr("1 order not sent yet")
                     : qsTr("%1 orders not sent yet").arg(OrderModel.pendingCount))
                  : qsTr("All orders sent")
            font.pixelSize: Theme.fontSizeBody
            color: OrderModel.pendingCount > 0 ? Theme.warning : Theme.textSecondary
        }

        ListView {
            id: orderList
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: Theme.spacingSmall
            clip: true
            model: OrderModel
            bottomMargin: Theme.spacingMedium
            ScrollIndicator.vertical: ScrollIndicator { }

            delegate: Rectangle {
                id: orderCard

                required property int orderId
                required property string createdAtText
                required property string statusText
                required property bool isSent
                required property string customerName
                required property string totalText
                required property int itemCount

                x: Theme.spacingMedium
                width: orderList.width - Theme.spacingMedium * 2
                implicitHeight: cardContent.implicitHeight + 24
                radius: Theme.radiusMedium
                color: cardTap.pressed ? Qt.rgba(Theme.border.r, Theme.border.g, Theme.border.b, 0.4) : Theme.surface
                border.color: Theme.border
                border.width: 1

                MouseArea {
                    id: cardTap
                    anchors.fill: parent
                    onClicked: orderDetailDialog.openFor(orderCard.orderId)
                }

                ColumnLayout {
                    id: cardContent
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 4

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Theme.spacingSmall

                        Label {
                            text: qsTr("Order #%1").arg(orderCard.orderId)
                            font.pixelSize: Theme.fontSizeBody
                            font.bold: true
                            color: Theme.textPrimary
                        }

                        StatusChip {
                            sent: orderCard.isSent
                            text: orderCard.statusText
                        }

                        Item { Layout.fillWidth: true }

                        Label {
                            text: orderCard.totalText
                            font.pixelSize: Theme.fontSizeSubtitle
                            font.bold: true
                            color: Theme.textPrimary
                        }
                    }

                    Label {
                        Layout.fillWidth: true
                        text: {
                            const items = orderCard.itemCount === 1 ? qsTr("1 item") : qsTr("%1 items").arg(orderCard.itemCount)
                            const parts = [orderCard.createdAtText, items]
                            if (orderCard.customerName.length > 0)
                                parts.push(orderCard.customerName)
                            return parts.join(" · ")
                        }
                        font.pixelSize: Theme.fontSizeCaption
                        color: Theme.textSecondary
                        elide: Text.ElideRight
                    }
                }
            }
        }
    }

    OrderDetailDialog {
        id: orderDetailDialog
        onReordered: count => page.reordered(count)
    }
}
