/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/
#include <QObject>
#include <QString>
#include <QtTest/QtTest>

//mdal
#include "mdal.h"

/**
 * \ingroup UnitTests
 * This is a unit test
 */
class TestCore : public QObject
{
    Q_OBJECT
  public:
    TestCore() = default;

  private slots:
    void initTestCase() {}// will be called before the first testfunction is executed.
    void cleanupTestCase() {}// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void test1();
};

void TestCore::test1()
{
  QVERIFY( true );
}


QTEST_MAIN( TestCore )
#include "testmdalcore.moc"

