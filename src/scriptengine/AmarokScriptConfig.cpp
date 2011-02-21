/****************************************************************************************
 * Copyright (c) 2008 Peter ZHOU <peterzhoulei@gmail.com>                               *
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

#include "AmarokScriptConfig.h"

#include <KConfigGroup>
#include <KSharedConfig>
#include <KGlobal>

namespace AmarokScript
{
    AmarokScriptConfig::AmarokScriptConfig( const QString& name, QObject *parent )
        : QObject( parent )
        , m_name( name )
    {}

    AmarokScriptConfig::~AmarokScriptConfig()
    {}

    QVariant AmarokScriptConfig::readConfig( const QString &name, const QVariant &defaultValue ) const
    {
        return KGlobal::config()->group( m_name ).readEntry( name, defaultValue );
    }

    QString AmarokScriptConfig::readConfig( const QString &name, const QString &defaultValue ) const
    {
        return KGlobal::config()->group( m_name ).readEntry( name, defaultValue );
    }

    void AmarokScriptConfig::writeConfig( const QString &name, const QVariant &content )
    {
        KGlobal::config()->group( m_name ).writeEntry( name, content );
    }

    void AmarokScriptConfig::writeConfig( const QString &name, const QString &content )
    {
        KGlobal::config()->group( m_name ).writeEntry( name, content );
    }
}

#include "AmarokScriptConfig.moc"
