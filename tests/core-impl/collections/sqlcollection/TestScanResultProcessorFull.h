/****************************************************************************************
 * Copyright (c) 2009 Maximilian Kossick <maximilian.kossick@googlemail.com>       *
 *                                                                                      *
 * This program is free software; you can redistribute it and/or modify it under        *
 * the terms of the GNU General Public License as published by the Free Software        *
 * Foundation; either version 2 of the License, or (at your option) any later           *
 * version.                                                                             *
 *                                                                                      *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
 *                                                                                      *
 * You should have received a copy of the GNU General Public License along with         *
 * this program.  If not, see <http://www.gnu.org/licenses/>.                           *
 ****************************************************************************************/

#ifndef TESTSCANRESULTPROCESSORFULL_H
#define TESTSCANRESULTPROCESSORFULL_H

#include <QtTest/QTest>
#include <QList>
#include <QPair>
#include <QString>
#include <QVariantMap>

#include <KTempDir>

class SqlStorage;

namespace Collections {
    class SqlCollection;
}

typedef QPair<QString, uint> DirMtime;

class TestScanResultProcessorFull : public QObject
{
    Q_OBJECT
public:
    TestScanResultProcessorFull();

private slots:
    void initTestCase();
    void cleanupTestCase();

    void cleanup();

    void testSingleInsert();
    void testAddDirectory();
    void testMerges();

    void testLargeInsert();
    void testIdentifyCompilationInMultipleDirectories();
    void testAFeatBDetectionInSingleDirectory();

private:

    QList<DirMtime> setupFileSystem( const QList<QVariantMap> &trackData );

    Collections::SqlCollection *m_collection;
    SqlStorage *m_storage;
    KTempDir *m_tmpDir;

};

#endif // TESTSCANRESULTPROCESSORFULL_H
