import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import WaveOrdering

Page {
    id: page
    background: Rectangle { color: Theme.background }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.spacingMedium
        spacing: Theme.spacingMedium

        TextField {
            id: searchField
            Layout.fillWidth: true
            placeholderText: qsTr("Search products or SKU…")
            onTextChanged: productModel.searchText = text
        }

        ListView {
            id: categoryRow
            Layout.fillWidth: true
            implicitHeight: 44
            orientation: ListView.Horizontal
            spacing: Theme.spacingSmall
            clip: true
            model: categoryModel

            delegate: CategoryChip {
                anchors.verticalCenter: parent ? parent.verticalCenter : undefined
                categoryName: model.name
                categoryId: model.categoryId
                selected: productModel.categoryFilter === model.categoryId
                onClicked: productModel.categoryFilter = model.categoryId
            }
        }

        ListView {
            id: productList
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: Theme.spacingSmall
            clip: true
            model: productModel

            delegate: ProductCard {
                width: productList.width
                productId: model.productId
                name: model.name
                sku: model.sku
                unit: model.unit
                price: model.price
                quantityInCart: model.quantityInCart
                onOpenDetails: detailDialog.openFor(
                    model.productId, model.name, model.sku, model.unit,
                    model.price, model.quantityInCart)
            }

            ScrollBar.vertical: ScrollBar { }

            Label {
                anchors.centerIn: parent
                visible: productList.count === 0
                text: qsTr("No products match your search.")
                color: Theme.textSecondary
                font.pixelSize: Theme.fontSizeBody
            }
        }
    }

    ProductDetailDialog {
        id: detailDialog
    }
}
