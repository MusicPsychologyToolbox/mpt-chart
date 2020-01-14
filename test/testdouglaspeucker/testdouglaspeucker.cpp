#include <QtTest>

#include "../../src/douglaspeucker.h"

class TestDouglasPeucker : public QObject
{
    Q_OBJECT

public:
    TestDouglasPeucker();
    ~TestDouglasPeucker();

private slots:
    void testDouglasPeucker_data();
    void testDouglasPeucker();
};

TestDouglasPeucker::TestDouglasPeucker()
{

}

TestDouglasPeucker::~TestDouglasPeucker()
{

}

void TestDouglasPeucker::testDouglasPeucker_data()
{
    QTest::addColumn<QVector<QPointF>>("input");
    QTest::addColumn<QVector<QPointF>>("result");

    QTest::addRow("empty") << QVector<QPointF>() << QVector<QPointF>();

    QTest::addRow("one") << QVector<QPointF>{ QPointF(1,1)} <<
                            QVector<QPointF>{ QPointF(1,1)};

    QTest::addRow("two") << QVector<QPointF>{ QPointF(1,1), QPointF(2,2) }
                         << QVector<QPointF>{ QPointF(1,1), QPointF(2,2) };

    QTest::addRow("line") << QVector<QPointF>{ QPointF(1,1), QPointF(2,1),
                                               QPointF(3,1), QPointF(4,1) }
                          << QVector<QPointF>{ QPointF(1,1), QPointF(4,1) };

    QTest::addRow("up") << QVector<QPointF>{ QPointF(-1,-1), QPointF(0,0),
                                             QPointF(1,1), QPointF(2,2) }
                        << QVector<QPointF>{ QPointF(-1,-1), QPointF(2,2) };

    QTest::addRow("down") << QVector<QPointF>{ QPointF(-1,1), QPointF(0,0),
                                               QPointF(1,-1), QPointF(2,-2) }
                          << QVector<QPointF>{ QPointF(-1,1), QPointF(2,-2) };

    QTest::addRow("up-down") << QVector<QPointF>{ QPointF(0.0, 1.0), QPointF(1.0 ,2.0),
                                                  QPointF(2.0, 3.0), QPointF(3.0 ,2.0),
                                                  QPointF(4.0, 1.0), QPointF(5.0 ,0.0) }
                             << QVector<QPointF>{ QPointF(0.0, 1.0), QPointF(2.0, 3.0),
                                                  QPointF(5.0, 0.0) };

    QTest::addRow("line") << QVector<QPointF>{ QPointF(-100,0), QPointF(-50, 0.2),
                                               QPointF(0,0), QPointF(50, -0.3),
                                               QPointF(100,0) }
                          << QVector<QPointF>{ QPointF(-100,0), QPointF(100, 0) };
}

void TestDouglasPeucker::testDouglasPeucker()
{
    QFETCH(QVector<QPointF>, input);
    QFETCH(QVector<QPointF>, result);

    QVector<QPointF> output;
    DouglasPeucker::douglasPeucker(input, 0.5, output);
    QCOMPARE(output, result);
}

QTEST_APPLESS_MAIN(TestDouglasPeucker)

#include "testdouglaspeucker.moc"
