pragma Singleton
import QtQuick

QtObject {
    // Brand palette: deep green (quality/fresh produce) with a warm amber
    // accent. Centralised here so the whole app can be re-skinned by
    // changing these values only.
    readonly property color primary: "#1B5E3C"
    readonly property color primaryDark: "#103D27"
    readonly property color primaryLight: "#3E8064"
    readonly property color accent: "#F2A93B"

    readonly property color background: "#F5F6F4"
    readonly property color surface: "#FFFFFF"
    readonly property color border: "#E2E5E1"

    readonly property color textPrimary: "#1A1A1A"
    readonly property color textSecondary: "#6B7280"
    readonly property color textOnPrimary: "#FFFFFF"

    readonly property color success: "#2E7D32"
    readonly property color danger: "#C62828"

    readonly property int radiusSmall: 8
    readonly property int radiusMedium: 14
    readonly property int radiusLarge: 20

    readonly property int spacingSmall: 8
    readonly property int spacingMedium: 16
    readonly property int spacingLarge: 24

    readonly property int fontSizeCaption: 12
    readonly property int fontSizeBody: 15
    readonly property int fontSizeSubtitle: 17
    readonly property int fontSizeTitle: 22
    readonly property int fontSizeHeadline: 28
}
