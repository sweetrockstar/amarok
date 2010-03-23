/***************************************************************************
 *   Copyright (c) 2009 Sven Krohlas <sven@getamarok.com>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef TESTTIMECODETRACKPROVIDER_H
#define TESTTIMECODETRACKPROVIDER_H

#include "TestBase.h"
#include "core/meta/impl/timecode/TimecodeTrackProvider.h"

#include <QtCore/QStringList>

class TestTimecodeTrackProvider : public TestBase
{
Q_OBJECT

public:
    TestTimecodeTrackProvider( const QStringList args, const QString &logPath );

private slots:
    void testPossiblyContainsTrack();
    void testTrackForUrl();
private:
    TimecodeTrackProvider m_testProvider;
};

#endif // TESTTIMECODETRACKPROVIDER_H
