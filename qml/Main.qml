import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import WaveOrdering

ApplicationWindow {
    id: window

    width: 412
    height: 915
    visible: true
    title: qsTr("Wave Ordering")

    Material.theme: Material.Light
    Material.primary: Theme.primary
    Material.accent: Theme.primaryLight
    Material.background: Theme.background
    Material.foreground: Theme.textPrimary

    readonly property int catalogueTab: 0
    readonly property int basketTab: 1
    readonly property int ordersTab: 2

    function showTab(index) {
        tabBar.currentIndex = index
    }

    function showToast(message) {
        toast.show(message)
    }

    header: ToolBar {
        height: 60
        background: Rectangle { color: Theme.primary }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: Theme.spacingMedium
            anchors.rightMargin: Theme.spacingMedium
            spacing: 12

            Image {
                source: "icons/logo.png"
                sourceSize: Qt.size(64, 64)
                Layout.preferredWidth: 32
                Layout.preferredHeight: 32
                fillMode: Image.PreserveAspectFit
            }

            Label {
                Layout.fillWidth: true
                text: qsTr("Wave Ordering")
                color: Theme.textOnPrimary
                font.pixelSize: Theme.fontSizeSubtitle + 1
                font.bold: true
                elide: Text.ElideRight
            }

            Label {
                text: qsTr("Sample catalogue")
                color: Theme.textOnPrimary
                opacity: 0.7
                font.pixelSize: Theme.fontSizeCaption
            }
        }
    }

    StackLayout {
        anchors.fill: parent
        currentIndex: tabBar.currentIndex

        CatalogPage {
        }

        CartPage {
            onBrowseRequested: window.showTab(window.catalogueTab)
            onOrderPlaced: orderId => sendConfirm.openFor(orderId)
        }

        OrdersPage {
            onBrowseRequested: window.showTab(window.catalogueTab)
            onReordered: count => {
                window.showTab(window.basketTab)
                window.showToast(count === 1
                                 ? qsTr("Added 1 product to your basket")
                                 : qsTr("Added %1 products to your basket").arg(count))
            }
        }
    }

    footer: TabBar {
        id: tabBar

        Material.background: Theme.surface
        Material.accent: Theme.primary
        Material.elevation: 8

        TabButton {
            text: qsTr("Catalogue")
            icon.source: "icons/catalogue.png"
            display: AbstractButton.TextUnderIcon
        }

        TabButton {
            id: basketTabButton
            text: qsTr("Basket")
            icon.source: "icons/basket.png"
            display: AbstractButton.TextUnderIcon

            Rectangle {
                visible: CartModel.itemCount > 0
                x: parent.width / 2 + 6
                y: 6
                width: Math.max(18, badgeLabel.implicitWidth + 10)
                height: 18
                radius: 9
                color: Theme.accent

                Label {
                    id: badgeLabel
                    anchors.centerIn: parent
                    text: CartModel.itemCount > 99 ? "99+" : CartModel.itemCount
                    color: "white"
                    font.pixelSize: 11
                    font.bold: true
                }
            }
        }

        TabButton {
            text: qsTr("Orders")
            icon.source: "icons/orders.png"
            display: AbstractButton.TextUnderIcon
        }
    }

    SendConfirmDialog {
        id: sendConfirm
        onKeptPending: orderId => window.showToast(
                           qsTr("Order #%1 saved as not sent. You can send it from Orders.").arg(orderId))
        onConfirmedSent: orderId => window.showToast(qsTr("Order #%1 marked as sent").arg(orderId))
    }

    Toast {
        id: toast
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: Theme.spacingMedium
    }

    Connections {
        target: AppController
        function onErrorOccurred(message) {
            window.showToast(message)
        }
    }
}
