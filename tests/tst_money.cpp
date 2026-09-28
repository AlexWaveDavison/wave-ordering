#include "money.h"

#include <QtTest>

class TestMoney : public QObject
{
    Q_OBJECT

private slots:
    void format_data()
    {
        QTest::addColumn<qint64>("pence");
        QTest::addColumn<QString>("expected");

        QTest::newRow("zero") << qint64(0) << QStringLiteral("£0.00");
        QTest::newRow("pence only") << qint64(5) << QStringLiteral("£0.05");
        QTest::newRow("pounds") << qint64(1450) << QStringLiteral("£14.50");
        QTest::newRow("thousands") << qint64(123456) << QStringLiteral("£1,234.56");
        QTest::newRow("negative") << qint64(-300) << QStringLiteral("-£3.00");
    }

    void format()
    {
        QFETCH(qint64, pence);
        QFETCH(QString, expected);
        QCOMPARE(Money::format(pence), expected);
    }

    void formatOrPoa()
    {
        QCOMPARE(Money::formatOrPoa(0), QStringLiteral("POA"));
        QCOMPARE(Money::formatOrPoa(999), QStringLiteral("£9.99"));
    }

    void fromPoundsRoundsCorrectly()
    {
        // 0.1 + 0.2 style floating-point noise must not leak into pence.
        QCOMPARE(Money::fromPounds(14.5), qint64(1450));
        QCOMPARE(Money::fromPounds(0.1 + 0.2), qint64(30));
        QCOMPARE(Money::fromPounds(19.99), qint64(1999));
        QCOMPARE(Money::fromPounds(1.005), qint64(100)); // 1.005 is really 1.00499999...
    }
};

QTEST_GUILESS_MAIN(TestMoney)
#include "tst_money.moc"
