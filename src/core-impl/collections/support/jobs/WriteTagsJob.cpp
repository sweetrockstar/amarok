/****************************************************************************************
 * Copyright (c) 2012 Matěj Laitl <matej@laitl.cz>                                      *
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

#include "WriteTagsJob.h"

#include "amarokconfig.h"
#include "MetaTagLib.h"

#include <QImage>

WriteTagsJob::WriteTagsJob( const QString &path, const Meta::FieldHash &changes, bool respectConfig )
    : QObject()
    , ThreadWeaver::Job()
    , m_path( path )
    , m_changes( changes )
    , m_respectConfig( respectConfig )
{
}

void WriteTagsJob::run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread *thread)
{
    Q_UNUSED(self);
    Q_UNUSED(thread);
    if( !AmarokConfig::writeBack() && m_respectConfig )
        return;

    Meta::Tag::writeTags( m_path, m_changes, AmarokConfig::writeBackStatistics() );

    if( m_changes.contains( Meta::valImage ) && ( AmarokConfig::writeBackCover() || !m_respectConfig ) )
        Meta::Tag::setEmbeddedCover( m_path, m_changes.value( Meta::valImage ).value<QImage>() );
}

void WriteTagsJob::defaultBegin(const ThreadWeaver::JobPointer& self, ThreadWeaver::Thread *thread)
{
    Q_EMIT started(self);
    ThreadWeaver::Job::defaultBegin(self, thread);
}

void WriteTagsJob::defaultEnd(const ThreadWeaver::JobPointer& self, ThreadWeaver::Thread *thread)
{
    ThreadWeaver::Job::defaultEnd(self, thread);
    if (!self->success()) {
        Q_EMIT failed(self);
    }
    Q_EMIT done(self);
}
