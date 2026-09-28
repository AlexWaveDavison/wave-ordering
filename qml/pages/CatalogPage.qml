import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import WaveOrdering

Page {
    id: page

    background: Rectangle { color: Theme.background }

    ColumnLayout {
        anchors.fill: parent
        anchors.topMargin: Theme.spacingMedium
        spacing: Theme.spacingSmall

        TextField {
            id: searchField
            Layout.fillWidth: true
            Layout.leftMargin: Theme.spacingMedium
            Layout.rightMargin: Theme.spacingMedium
            placeholderText: qsTr("Search products or SKU")
            leftPadding: 40
            rightPadding: clearButton.visible ? 40 : 12
            inputMethodHints: Qt.ImhNoPredictiveText
            onTextChanged: ProductModel.searchText = text

            Image {
                anchors.left: parent.left
                anchors.leftMargin: 12
                anchors.verticalCenter: parent.verticalCenter
                source: "../icons/search.png"
                sourceSize: Qt.size(48, 48)
                width: 18
                height: 18
                opacity: 0.55
            }

            ToolButton {
                id: clearButton
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                visible: searchField.text.length > 0
                text: "✕"
                font.pixelSize: 13
                onClicked: searchField.clear()
                Accessible.name: qsTr("Clear search")
            }
        }

        ListView {
            id: categoryRow
            Layout.fillWidth: true
            Layout.preferredHeight: 46
            orientation: ListView.Horizontal
            spacing: Theme.spacingSmall
            leftMargin: Theme.spacingMedium
            rightMargin: Theme.spacingMedium
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            model: CategoryModel

            delegate: CategoryChip {
                required property int categoryId
                required property string name
                anchors.verticalCenter: parent ? parent.verticalCenter : undefined
                categoryName: name
                selected: ProductModel.categoryFilter === categoryId
                onClicked: ProductModel.categoryFilter = categoryId
            }
        }

        Label {
            Layout.leftMargin: Theme.spacingMedium
            visible: ProductModel.count > 0
            text: ProductModel.count === 1 ? qsTr("1 product") : qsTr("%1 products").arg(ProductModel.count)
            font.pixelSize: Theme.fontSizeCaption
            color: Theme.textSecondary
        }

        ListView {
            id: productList
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: Theme.spacingSmall
            clip: true
            model: ProductModel
            topMargin: 4
            bottomMargin: Theme.spacingMedium
            ScrollIndicator.vertical: ScrollIndicator { }

            delegate: ProductCard {
                id: productCard
                x: Theme.spacingMedium
                width: productList.width - Theme.spacingMedium * 2
                onOpenDetails: detailDialog.openFor(productCard)
            }

            EmptyState {
                anchors.centerIn: parent
                visible: productList.count === 0
                iconSource: "../icons/search.png"
                title: qsTr("No products found")
                message: qsTr("Try a different search or category.")
                actionText: qsTr("Clear filters")
                onActionTriggered: {
                    searchField.clear()
                    ProductModel.categoryFilter = -1
                }
            }
        }
    }

    ProductDetailDialog {
        id: detailDialog
    }
}
