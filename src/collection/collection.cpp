/*
 *  Copyright (c) 2007 Maximilian Kossick <maximilian.kossick@googlemail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "collection.h"

#include "CollectionLocation.h"
#include "meta/meta.h"

CollectionFactory::CollectionFactory() : Amarok::Plugin()
{
}

CollectionFactory::~CollectionFactory()
{
}


Collection::Collection()
    : QObject()
{
}

Collection::~Collection()
{
}

bool
Collection::possiblyContainsTrack( const KUrl &url ) const
{
    Q_UNUSED( url )
    return false;
}

Meta::TrackPtr
Collection::trackForUrl( const KUrl &url )
{
    Q_UNUSED( url )
    return Meta::TrackPtr();
}

CollectionLocation*
Collection::location() const
{
    return new CollectionLocation();
}

#include "collection.moc"
