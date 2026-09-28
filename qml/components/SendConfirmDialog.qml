import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import WaveOrdering

// Shown straight after WhatsApp opens. The app can't see whether the message
// was actually sent, so it asks, and only then marks the order as sent.
Dialog {
    id: dialog

    property int orderId: -1

    signal confirmedSent(int orderId)
    signal keptPending(int orderId)

    function openFor(id) {
        orderId = id
        open()
    }

    modal: true
    closePolicy: Popup.NoAutoClose
    anchors.centerIn: parent
    width: Math.min(parent ? parent.width - 32 : 360, 400)
    padding: Theme.spacingLarge

    contentItem: ColumnLayout {
        spacing: Theme.spacingMedium

        Label {
            Layout.fillWidth: true
            text: qsTr("Did you send order #%1?").arg(dialog.orderId)
            wrapMode: Text.WordWrap
            font.pixelSize: Theme.fontSizeSubtitle
            font.bold: true
            color: Theme.textPrimary
        }

        Label {
            Layout.fillWidth: true
            text: qsTr("Your order is saved and WhatsApp has opened with the summary. "
                       + "Once you've sent it to your rep, mark it as sent so you can keep track.")
            wrapMode: Text.WordWrap
            font.pixelSize: Theme.fontSizeBody
            color: Theme.textSecondary
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.topMargin: Theme.spacingSmall
            spacing: 4

            Button {
                Layout.fillWidth: true
                highlighted: true
                text: qsTr("Yes, it's sent")
                onClicked: {
                    const id = dialog.orderId
                    dialog.close()
                    if (AppController.markOrderSent(id))
                        dialog.confirmedSent(id)
                }
            }

            Button {
                Layout.fillWidth: true
                flat: true
                text: qsTr("Open WhatsApp again")
                onClicked: AppController.shareOrderViaWhatsApp(dialog.orderId)
            }

            Button {
                Layout.fillWidth: true
                flat: true
                text: qsTr("Not yet")
                onClicked: {
                    const id = dialog.orderId
                    dialog.close()
                    dialog.keptPending(id)
                }
            }
        }
    }
}
