import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import WaveOrdering

ApplicationWindow {
    id: window
    visible: true
    width: 412
    height: 915
    title: qsTr("Wave Ordering")

    Material.theme: Material.Light
    Material.primary: Theme.primary
    Material.accent: Theme.accent
    Material.background: Theme.background

    header: ToolBar {
        height: 64
        background: Rectangle { color: Theme.primary }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: Theme.spacingMedium
            anchors.rightMargin: Theme.spacingMedium

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 0

                Label {
                    text: qsTr("Wave Ordering")
                    color: Theme.textOnPrimary
                    font.pixelSize: Theme.fontSizeTitle
                    font.bold: true
                }
                Label {
                    text: qsTr("Wholesale Ordering Demo")
                    color: Theme.textOnPrimary
                    opacity: 0.78
                    font.pixelSize: Theme.fontSizeCaption
                }
            }
        }
    }

    StackLayout {
        anchors.fill: parent
        currentIndex: tabBar.currentIndex

        CatalogPage { }
        CartPage { }
        OrdersPage { }
    }

    footer: TabBar {
        id: tabBar
        Material.elevation: 8

        TabButton {
            text: qsTr("Catalog")
        }
        TabButton {
            text: cartModel.itemCount > 0
                  ? qsTr("Basket (%1)").arg(cartModel.itemCount)
                  : qsTr("Basket")
        }
        TabButton {
            text: qsTr("Orders")
        }
    }
}
