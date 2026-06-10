#pragma once

#include <QColor>
#include <QObject>
#include <QQmlEngine>

// Centralised brand palette/spacing/typography, exposed to QML as the
// `Theme` singleton (import WaveOrdering). Implemented in C++ so it is
// always available regardless of QML singleton registration quirks.
class Theme : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QColor primary READ primary CONSTANT)
    Q_PROPERTY(QColor primaryDark READ primaryDark CONSTANT)
    Q_PROPERTY(QColor primaryLight READ primaryLight CONSTANT)
    Q_PROPERTY(QColor accent READ accent CONSTANT)

    Q_PROPERTY(QColor background READ background CONSTANT)
    Q_PROPERTY(QColor surface READ surface CONSTANT)
    Q_PROPERTY(QColor border READ border CONSTANT)

    Q_PROPERTY(QColor textPrimary READ textPrimary CONSTANT)
    Q_PROPERTY(QColor textSecondary READ textSecondary CONSTANT)
    Q_PROPERTY(QColor textOnPrimary READ textOnPrimary CONSTANT)

    Q_PROPERTY(QColor success READ success CONSTANT)
    Q_PROPERTY(QColor danger READ danger CONSTANT)

    Q_PROPERTY(int radiusSmall READ radiusSmall CONSTANT)
    Q_PROPERTY(int radiusMedium READ radiusMedium CONSTANT)
    Q_PROPERTY(int radiusLarge READ radiusLarge CONSTANT)

    Q_PROPERTY(int spacingSmall READ spacingSmall CONSTANT)
    Q_PROPERTY(int spacingMedium READ spacingMedium CONSTANT)
    Q_PROPERTY(int spacingLarge READ spacingLarge CONSTANT)

    Q_PROPERTY(int fontSizeCaption READ fontSizeCaption CONSTANT)
    Q_PROPERTY(int fontSizeBody READ fontSizeBody CONSTANT)
    Q_PROPERTY(int fontSizeSubtitle READ fontSizeSubtitle CONSTANT)
    Q_PROPERTY(int fontSizeTitle READ fontSizeTitle CONSTANT)
    Q_PROPERTY(int fontSizeHeadline READ fontSizeHeadline CONSTANT)

public:
    explicit Theme(QObject *parent = nullptr) : QObject(parent) {}

    // Brand palette inspired by "The Great Wave": deep indigo/navy, ocean
    // blue, warm cream foam, and a red sun accent.
    QColor primary() const { return QColor("#16263F"); }
    QColor primaryDark() const { return QColor("#0B1626"); }
    QColor primaryLight() const { return QColor("#3D6FA3"); }
    QColor accent() const { return QColor("#C0392B"); }

    QColor background() const { return QColor("#F6F3EA"); }
    QColor surface() const { return QColor("#FFFFFF"); }
    QColor border() const { return QColor("#E3E0D5"); }

    QColor textPrimary() const { return QColor("#1B2733"); }
    QColor textSecondary() const { return QColor("#67737F"); }
    QColor textOnPrimary() const { return QColor("#F6F3EA"); }

    QColor success() const { return QColor("#2F7D6B"); }
    QColor danger() const { return QColor("#C0392B"); }

    int radiusSmall() const { return 8; }
    int radiusMedium() const { return 14; }
    int radiusLarge() const { return 20; }

    int spacingSmall() const { return 8; }
    int spacingMedium() const { return 16; }
    int spacingLarge() const { return 24; }

    int fontSizeCaption() const { return 12; }
    int fontSizeBody() const { return 15; }
    int fontSizeSubtitle() const { return 17; }
    int fontSizeTitle() const { return 22; }
    int fontSizeHeadline() const { return 28; }
};
